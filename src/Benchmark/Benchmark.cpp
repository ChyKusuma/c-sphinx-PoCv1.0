// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#include <iostream>
#include <fstream>
#include <regex>
#include <set>
#include <map>
#include <chrono>
#include <functional>
#include <vector>
#include <filesystem>

#include <bench/bench.hpp>

#include <test/util/setup_common.h>
#include <util/fs.h>
#include <util/string.h>


// Importing file system library
namespace fs = std::filesystem;

// Defining the benchmark namespace
namespace benchmark {

// Importing chrono_literals namespace for easy access to chrono literals
using namespace std::chrono_literals;

// Global function for logging (empty function)
const std::function<void(const std::string&)> G_TEST_LOG_FUN{};

// Global function for getting command line arguments (empty function)
const std::function<std::vector<const char*>()> G_TEST_COMMAND_LINE_ARGUMENTS{};

// Enum defining priority levels
enum class PriorityLevel : uint8_t {
    HIGH = 1,
    LOW = 2,
};

// Map associating priority labels with their respective PriorityLevel
std::map<std::string, PriorityLevel> map_label_priority = {
    {"high", PriorityLevel::HIGH},
    {"low", PriorityLevel::LOW},
    {"all", static_cast<PriorityLevel>(0xff)}
};

// Function to list priority labels sorted by priority
std::string ListPriorities() {
    auto sort_by_priority = [](const auto& a, const auto& b) { return a.second < b.second; };
    std::set<std::pair<std::string, PriorityLevel>, decltype(sort_by_priority)> sorted_priorities(map_label_priority.begin(), map_label_priority.end(), sort_by_priority);
    std::string result;
    for (const auto& entry : sorted_priorities) {
        result += entry.first + ",";
    }
    return result;
}

// Function to convert a string to its corresponding priority level
uint8_t StringToPriority(const std::string& str) {
    auto it = map_label_priority.find(str);
    if (it == map_label_priority.end())
        throw std::runtime_error("Unknown priority level " + str);
    return static_cast<uint8_t>(it->second);
}

// Definition of the BenchRunner class
class BenchRunner {
public:
    using BenchFunction = std::function<void(void)>;

    // Definition of a bench struct containing name, function, and priority level
    struct Bench {
        std::string name;
        BenchFunction func;
        PriorityLevel priority_level;
    };

    // Definition of the BenchmarkMap type
    using BenchmarkMap = std::map<std::string, Bench>;

    // Static function to access benchmarks map
    static BenchmarkMap& benchmarks() {
        static BenchmarkMap benchmarks_map;
        return benchmarks_map;
    }

    // Constructor for BenchRunner class
    BenchRunner(std::string name, BenchFunction func, PriorityLevel level) {
        benchmarks().insert({name, {name, func, level}});
    }

    // Function to run all benchmarks based on provided arguments
    void RunAll(const Args& args);

private:
    // Function to generate template results
    void GenerateTemplateResults(const std::vector<Bench>& benchmarkResults, const fs::path& file, const char* tpl);

    // Regular expression filter
    std::regex reFilter;
};

// Implementation of RunAll function
void BenchRunner::RunAll(const Args& args) {
    std::vector<Bench> benchmarkResults;
    for (const auto& [name, bench] : benchmarks()) {
        if (!(bench.priority_level & args.priority))
            continue;

        if (!std::regex_match(name, reFilter))
            continue;

        if (args.is_list_only) {
            std::cout << name << std::endl;
            continue;
        }

        bench.func();

        benchmarkResults.push_back(bench);
    }

    GenerateTemplateResults(benchmarkResults, args.output_csv, "# Benchmark, evals, iterations, total, min, max, median\n"
                                                               "{{#result}}{{name}}, {{epochs}}, {{average(iterations)}}, {{sumProduct(iterations, elapsed)}}, {{minimum(elapsed)}}, {{maximum(elapsed)}}, {{median(elapsed)}}\n"
                                                               "{{/result}}");
    GenerateTemplateResults(benchmarkResults, args.output_json, ankerl::nanobench::templates::json());
}

// Implementation of GenerateTemplateResults function
void BenchRunner::GenerateTemplateResults(const std::vector<Bench>& benchmarkResults, const fs::path& file, const char* tpl) {
    if (benchmarkResults.empty() || file.empty()) {
        // nothing to write, bail out
        return;
    }
    std::ofstream fout{file};
    if (fout.is_open()) {
        // ankerl::nanobench::render(tpl, benchmarkResults, fout);
        std::cout << "Created " << file << std::endl;
    } else {
        std::cout << "Could not write to file " << file << std::endl;
    }
}

} // namespace benchmark

