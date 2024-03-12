// Copyright (c) [2023] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <map>
#include <mutex>
#include <set>
#include <system_error>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <Sync.hpp>

#include <logging.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <util/threadnames.h>

#ifdef DEBUG_LOCKORDER
// Early deadlock detection.
// Problem being solved:
//    Thread 1 locks A, then B, then C
//    Thread 2 locks D, then C, then A
//     --> may result in deadlock between the two threads, depending on when they run.
// Solution implemented here:
// Keep track of pairs of locks: (A before B), (A before C), etc.
// Complain if any thread tries to lock in a different order.
struct LockLocation {
    LockLocation(const char* name, const char* file, int line, bool tryIn, const std::string& threadName)
        : tryFlag(tryIn),
          mutexName(name),
          sourceFile(file),
          threadName(threadName),
          sourceLine(line) {}

    std::string ToString() const
    {
        return strprintf(
            "'%s' in %s:%s%s (in thread '%s')",
            mutexName, sourceFile, sourceLine, (tryFlag ? " (TRY)" : ""), threadName);
    }

    std::string Name() const
    {
        return mutexName;
    }

private:
    bool tryFlag;
    std::string mutexName;
    std::string sourceFile;
    const std::string& threadName;
    int sourceLine;
};

using LockStackItem = std::pair<void*, LockLocation>;
using LockStack = std::vector<LockStackItem>;
using LockStacks = std::unordered_map<std::thread::id, LockStack>;

using LockPair = std::pair<void*, void*>;
using LockOrders = std::map<LockPair, LockStack>;
using InvLockOrders = std::set<LockPair>;

struct LockData {
    LockStacks lockStacks;
    LockOrders lockOrders;
    InvLockOrders invLockOrders;
    std::mutex mutex;
};

LockData& GetLockData() {
    static LockData& lockData = *new LockData();
    return lockData;
}

static void PotentialDeadlockDetected(const LockPair& mismatch, const LockStack& s1, const LockStack& s2)
{
    LogPrintf("POTENTIAL DEADLOCK DETECTED\n");
    LogPrintf("Previous lock order was:\n");
    for (const LockStackItem& i : s1) {
        std::string prefix{};
        if (i.first == mismatch.first) {
            prefix = " (1)";
        }
        if (i.first == mismatch.second) {
            prefix = " (2)";
        }
        LogPrintf("%s %s\n", prefix, i.second.ToString());
    }

    std::string mutexA, mutexB;
    LogPrintf("Current lock order is:\n");
    for (const LockStackItem& i : s2) {
        std::string prefix{};
        if (i.first == mismatch.first) {
            prefix = " (1)";
            mutexA = i.second.Name();
        }
        if (i.first == mismatch.second) {
            prefix = " (2)";
            mutexB = i.second.Name();
        }
        LogPrintf("%s %s\n", prefix, i.second.ToString());
    }
    if (g_debug_lockorder_abort) {
        tfm::format(std::cerr, "Assertion failed: detected inconsistent lock order for %s, details in debug log.\n", s2.back().second.ToString());
        abort();
    }
    throw std::logic_error(strprintf("potential deadlock detected: %s -> %s -> %s", mutexB, mutexA, mutexB));
}

static void DoubleLockDetected(const void* mutex, const LockStack& lockStack)
{
    LogPrintf("DOUBLE LOCK DETECTED\n");
    LogPrintf("Lock order:\n");
    for (const LockStackItem& i : lockStack) {
        std::string prefix{};
        if (i.first == mutex) {
            prefix = " (*)";
        }
        LogPrintf("%s %s\n", prefix, i.second.ToString());
    }
    if (g_debug_lockorder_abort) {
        tfm::format(std::cerr,
                    "Assertion failed: detected double lock for %s, details in debug log.\n",
                    lockStack.back().second.ToString());
        abort();
    }
    throw std::logic_error("double lock detected");
}

template <typename MutexType>
static void PushLock(MutexType* c, const LockLocation& lockLocation)
{
    constexpr bool isRecursiveMutex =
        std::is_base_of<RecursiveMutex, MutexType>::value ||
        std::is_base_of<std::recursive_mutex, MutexType>::value;

    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);

    LockStack& lockStack = lockData.lockStacks[std::this_thread::get_id()];
    lockStack.emplace_back(c, lockLocation);
    for (size_t j = 0; j < lockStack.size() - 1; ++j) {
        const LockStackItem& i = lockStack[j];
        if (i.first == c) {
            if (isRecursiveMutex) {
                break;
            }
            auto lockStackCopy = lockStack;
            lockStack.pop_back();
            DoubleLockDetected(c, lockStackCopy);
            // DoubleLockDetected() does not return.
        }

        const LockPair p1 = std::make_pair(i.first, c);
        if (lockData.lockOrders.count(p1))
            continue;

        const LockPair p2 = std::make_pair(c, i.first);
        if (lockData.lockOrders.count(p2)) {
            auto lockStackCopy = lockStack;
            lockStack.pop_back();
            PotentialDeadlockDetected(p1, lockData.lockOrders[p2], lockStackCopy);
            // PotentialDeadlockDetected() does not return.
        }

        lockData.lockOrders.emplace(p1, lockStack);
        lockData.invLockOrders.insert(p2);
    }
}

static void PopLock()
{
    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);

    LockStack& lockStack = lockData.lockStacks[std::this_thread::get_id()];
    lockStack.pop_back();
    if (lockStack.empty()) {
        lockData.lockStacks.erase(std::this_thread::get_id());
    }
}

template <typename MutexType>
void EnterCritical(const char* name, const char* file, int line, MutexType* cs, bool tryFlag)
{
    PushLock(cs, LockLocation(name, file, line, tryFlag, util::ThreadGetInternalName()));
}

void CheckLastCritical(void* cs, std::string& lockName, const char* guardName, const char* file, int line)
{
    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);

    const LockStack& lockStack = lockData.lockStacks[std::this_thread::get_id()];
    if (!lockStack.empty()) {
        const auto& lastLock = lockStack.back();
        if (lastLock.first == cs) {
            lockName = lastLock.second.Name();
            return;
        }
    }

    LogPrintf("INCONSISTENT LOCK ORDER DETECTED\n");
    LogPrintf("Current lock order (least recent first) is:\n");
    for (const LockStackItem& i : lockStack) {
        LogPrintf(" %s\n", i.second.ToString());
    }
    if (g_debug_lockorder_abort) {
        tfm::format(std::cerr, "%s:%s %s was not most recent critical section locked, details in debug log.\n", file, line, guardName);
        abort();
    }
    throw std::logic_error(strprintf("%s was not most recent critical section locked", guardName));
}

void LeaveCritical()
{
    PopLock();
}

std::string LocksHeld()
{
    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);

    const LockStack& lockStack = lockData.lockStacks[std::this_thread::get_id()];
    std::string result;
    for (const LockStackItem& i : lockStack)
        result += i.second.ToString() + std::string("\n");
    return result;
}

static bool LockHeld(void* mutex)
{
    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);

    const LockStack& lockStack = lockData.lockStacks[std::this_thread::get_id()];
    for (const LockStackItem& i : lockStack) {
        if (i.first == mutex) return true;
    }

    return false;
}

template <typename MutexType>
void AssertLockHeldInternal(const char* name, const char* file, int line, MutexType* cs)
{
    if (LockHeld(cs)) return;
    tfm::format(std::cerr, "Assertion failed: lock %s not held in %s:%i; locks held:\n%s", name, file, line, LocksHeld());
    abort();
}

template <typename MutexType>
void AssertLockNotHeldInternal(const char* name, const char* file, int line, MutexType* cs)
{
    if (!LockHeld(cs)) return;
    tfm::format(std::cerr, "Assertion failed: lock %s held in %s:%i; locks held:\n%s", name, file, line, LocksHeld());
    abort();
}

void DeleteLock(void* cs)
{
    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);
    const LockPair item = std::make_pair(cs, nullptr);
    LockOrders::iterator it = lockData.lockOrders.lower_bound(item);
    while (it != lockData.lockOrders.end() && it->first.first == cs) {
        const LockPair invItem = std::make_pair(it->first.second, it->first.first);
        lockData.invLockOrders.erase(invItem);
        lockData.lockOrders.erase(it++);
    }
    InvLockOrders::iterator invIt = lockData.invLockOrders.lower_bound(item);
    while (invIt != lockData.invLockOrders.end() && invIt->first == cs) {
        const LockPair invInvItem = std::make_pair(invIt->second, invIt->first);
        lockData.lockOrders.erase(invInvItem);
        lockData.invLockOrders.erase(invIt++);
    }
}

bool LockStackEmpty()
{
    LockData& lockData = GetLockData();
    std::lock_guard<std::mutex> lock(lockData.mutex);
    const auto it = lockData.lockStacks.find(std::this_thread::get_id());
    if (it == lockData.lockStacks.end()) {
        return true;
    }
    return it->second.empty();
}

bool g_debug_lockorder_abort = true;

#endif

