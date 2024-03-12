// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#include <array>
#include <vector>

#include <benchmark/bench.hpp>
#include <base58.hpp>


// Function to benchmark the Base58 encoding operation
static void Base58Encode(benchmark::Bench& bench)
{
    // Static array representing a 32-byte buffer
    static const std::array<unsigned char, 32> buff = {
        {
            // Data bytes for encoding
            17, 79, 8, 99, 150, 189, 208, 162, 22, 23, 203, 163, 36, 58, 147,
            227, 139, 2, 215, 100, 91, 38, 11, 141, 253, 40, 117, 21, 16, 90,
            200, 24
        }
    };
    
    // Benchmarking batch operation for each byte in the buffer
    bench.batch(buff.size()).unit("byte").run([&] {
        // Call the Base58 encoding function
        EncodeBase58(buff);
    });
}

// Function to benchmark the Base58Check encoding operation
static void Base58CheckEncode(benchmark::Bench& bench)
{
    // Static array representing a 32-byte buffer
    static const std::array<unsigned char, 32> buff = {
        {
            // Data bytes for encoding
            17, 79, 8, 99, 150, 189, 208, 162, 22, 23, 203, 163, 36, 58, 147,
            227, 139, 2, 215, 100, 91, 38, 11, 141, 253, 40, 117, 21, 16, 90,
            200, 24
        }
    };
    
    // Benchmarking batch operation for each byte in the buffer
    bench.batch(buff.size()).unit("byte").run([&] {
        // Call the Base58Check encoding function
        EncodeBase58Check(buff);
    });
}

// Function to benchmark the Base58 decoding operation
static void Base58Decode(benchmark::Bench& bench)
{
    // Base58 encoded string to be decoded
    const char* addr = "17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem";
    std::vector<unsigned char> vch;
    
    // Benchmarking batch operation for each byte in the address string
    bench.batch(std::strlen(addr)).unit("byte").run([&] {
        // Call the Base58 decoding function
        (void) DecodeBase58(addr, vch, 64);
    });
}

// Benchmarking tasks for Base58 encoding, Base58Check encoding, and Base58 decoding
BENCHMARK(Base58Encode, benchmark::PriorityLevel::HIGH);
BENCHMARK(Base58CheckEncode, benchmark::PriorityLevel::HIGH);
BENCHMARK(Base58Decode, benchmark::PriorityLevel::HIGH);

