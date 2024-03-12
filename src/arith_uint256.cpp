// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#include <uint256.hpp>
#include <arith_uint256.hpp>
#include <crypto/common.h>

template <size_t N>
class uint256 {
private:
    // Store 256-bit integer as array of N 64-bit integers
    std::array<uint64_t, N> data;

public:
    // Default constructor
    uint256() : data({0}) {}

    // Constructor from unsigned long long
    uint256(uint64_t value) : data({value}) {}

    // Addition
    uint256 operator+(const uint256& other) const {
        uint256 result;
        uint64_t carry = 0;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] + other.data[i] + carry;
            carry = (result.data[i] < data[i] || (result.data[i] == data[i] && carry > 0));
        }
        return result;
    }

    // Subtraction
    uint256 operator-(const uint256& other) const {
        uint256 result;
        uint64_t borrow = 0;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] - other.data[i] - borrow;
            borrow = (result.data[i] > data[i] || (result.data[i] == data[i] && borrow > 0));
        }
        return result;
    }

    // Bitwise AND
    uint256 operator&(const uint256& other) const {
        uint256 result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] & other.data[i];
        }
        return result;
    }

    // Bitwise OR
    uint256 operator|(const uint256& other) const {
        uint256 result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] | other.data[i];
        }
        return result;
    }

    // Bitwise XOR
    uint256 operator^(const uint256& other) const {
        uint256 result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] ^ other.data[i];
        }
        return result;
    }

    // Comparison operators
    bool operator==(const uint256& other) const {
        return data == other.data;
    }

    bool operator!=(const uint256& other) const {
        return !(*this == other);
    }

    bool operator<(const uint256& other) const {
        for (size_t i = N - 1; i != static_cast<size_t>(-1); --i) {
            if (data[i] < other.data[i]) return true;
            if (data[i] > other.data[i]) return false;
        }
        return false;
    }

    bool operator>(const uint256& other) const {
        return other < *this;
    }

    bool operator<=(const uint256& other) const {
        return !(other < *this);
    }

    bool operator>=(const uint256& other) const {
        return !(*this < other);
    }

    // Set from compact representation
    uint256& SetCompact(uint32_t nCompact, bool* pfNegative, bool* pfOverflow) {
        int nSize = nCompact >> 24;
        uint32_t nWord = nCompact & 0x007fffff;
        if (nSize <= 3) {
            nWord >>= 8 * (3 - nSize);
            data[0] = nWord;
        } else {
            data[N - 1] = nWord;
            for (size_t i = N - 2; i >= N - nSize; --i) {
                data[i] = 0;
            }
        }
        if (pfNegative)
            *pfNegative = nWord != 0 && (nCompact & 0x00800000) != 0;
        if (pfOverflow)
            *pfOverflow = nWord != 0 && ((nSize > 34) || (nWord > 0xff && nSize > 33) || (nWord > 0xffff && nSize > 32));
        return *this;
    }

    // Get compact representation
    uint32_t GetCompact(bool fNegative) const {
        int nSize = 0;
        for (size_t i = N - 1; i != static_cast<size_t>(-1); --i) {
            if (data[i] != 0) {
                nSize = (i + 1) * 8;
                break;
            }
        }
        uint32_t nCompact = 0;
        if (nSize <= 3) {
            nCompact = data[0] << 8 * (3 - nSize);
        } else {
            uint256 temp = *this >> 8 * (nSize - 3);
            nCompact = temp.data[N - 1];
        }
        if (fNegative && (nCompact & 0x00800000)) {
            nCompact >>= 8;
            nSize++;
        }
        assert((nCompact & ~0x007fffffU) == 0);
        assert(nSize < 256);
        nCompact |= nSize << 24;
        nCompact |= (fNegative && (nCompact & 0x007fffff) ? 0x00800000 : 0);
        return nCompact;
    }
};

// Example usage
int main() {
    uint256<4> a(123456789), b(987654321);
    uint256<4> c = a + b;
    std::cout << "a + b = " << c << std::endl;
    return 0;
}
