// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#include <vector>
#include <string>
#include <Hash.hpp> // Include the header for SPHINXHash
#include <Consensus/Merkle.hpp>
#include "Benchmark.hpp"
#include "Merkle_root.hpp"

namespace SPHINXMerkle {

    // Function to compute the Merkle root of a set of transaction hashes
    std::string ComputeMerkleRoot(const std::vector<std::string>& transactionHashes) {
        // Initialize mutated flag
        bool mutated = false;
        // Create a copy of transactionHashes
        std::vector<std::string> hashes = transactionHashes;

        // Iteratively compute Merkle root until only one hash remains
        while (hashes.size() > 1) {
            // Duplicate the last hash if the count is odd
            if (hashes.size() % 2 != 0) {
                hashes.push_back(hashes.back());
            }

            // Create a new vector to store combined hashes
            std::vector<std::string> new_hashes;
            // Combine and hash pairs of hashes
            for (size_t i = 0; i < hashes.size(); i += 2) {
                // Concatenate the two hashes
                std::string combinedHash = hashes[i] + hashes[i + 1];
                // Compute the hash of the concatenated string
                std::string hash = SPHINXHash::SPHINX_256(combinedHash);
                // Store the computed hash in the new vector
                new_hashes.push_back(hash);
            }

            // Update the hashes vector with the new_hashes vector
            hashes = new_hashes;
        }

        // Return the final Merkle root
        return hashes[0];
    }

    // Function to benchmark the computation of Merkle Root
    static void BM_MerkleRoot(benchmark::State& state) {
        // Generate random data for leaves
        FastRandomContext rng(true);
        std::vector<std::string> leaves;
        leaves.resize(9001);
        for (auto& item : leaves) {
            item = rng.rand256().ToString();
        }

        // Benchmark the Merkle Root computation
        while (state.KeepRunning()) {
            // Simulate mutation
            bool mutation = false;
            // Compute the Merkle Root
            std::string root = ComputeMerkleRoot(leaves);
            // Update leaves with the computed root
            leaves[mutation] = root;
        }
    }

    // Register the benchmark function
    BENCHMARK(BM_MerkleRoot);

    // Define main function for benchmarking
    BENCHMARK_MAIN();

} // namespace SPHINXMerkle



