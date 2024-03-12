#ifndef SPHINX_BENCH_BENCH_H
#define SPHINX_BENCH_BENCH_H

#include <util/fs.h>
#include <util/macros.h>
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <bench/nanobench.h>

// Defining the benchmark namespace
namespace benchmark {

// Importing the Bench class from ankerl::nanobench namespace
using ankerl::nanobench::Bench;

// Enum defining priority levels
enum class PriorityLevel : uint8_t {
    LOW = 1 << 0,
    HIGH = 1 << 2,
};

// Declaration of functions for listing priorities and converting string to priority level
std::string ListPriorities();
uint8_t StringToPriority(const std::string& str);

// Structure representing arguments for benchmarks
struct Args {
    bool is_list_only; // Flag indicating if only list of benchmarks should be displayed
    bool sanity_check; // Flag indicating if sanity check should be performed
    std::chrono::milliseconds min_time; // Minimum time for benchmark execution
    std::vector<double> asymptote; // Vector of asymptote values for benchmarks
    fs::path output_csv; // Path to output CSV file
    fs::path output_json; // Path to output JSON file
    std::string regex_filter; // Regular expression filter for benchmarks
    uint8_t priority; // Priority level for benchmark execution
};

// Declaration of the BenchRunner class
class BenchRunner {
    // Alias for benchmark function
    using BenchFunction = std::function<void(Bench&)>;

    // Structure representing data associated with each benchmark
    struct BenchmarkData {
        BenchFunction function; // Benchmark function
        PriorityLevel priority; // Priority level
    };

    // Static function to access benchmarks map
    static std::map<std::string, BenchmarkData>& GetBenchmarks();

public:
    // Constructor for BenchRunner class
    BenchRunner(std::string name, BenchFunction func, PriorityLevel level);

    // Static function to run all benchmarks
    static void RunAll(const Args& args);
};

} // namespace benchmark

// Macro for defining benchmarks
#define BENCHMARK(name, priority_level) \
    benchmark::BenchRunner PASTE2(bench_, PASTE2(__LINE__, name))(STRINGIZE(name), name, priority_level);

#endif // SPHINX_BENCH_BENCH_H
