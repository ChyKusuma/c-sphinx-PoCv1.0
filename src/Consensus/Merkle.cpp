// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#include <iostream>
#include <vector>
#include <string>

#include <Hash.hpp>
#include "Merkle.hpp"


// Compute the Merkle root of a list of hashes
uint256 ComputeMerkleRoot(std::vector<uint256> hashes, bool* mutated) {
    bool mutation = false;
    // Loop until there's only one hash left
    while (hashes.size() > 1) {
        // Check for mutation if needed
        if (mutated) {
            for (size_t pos = 0; pos + 1 < hashes.size(); pos += 2) {
                if (hashes[pos] == hashes[pos + 1]) mutation = true;
            }
        }
        // If the number of hashes is odd, duplicate the last one
        if (hashes.size() & 1) {
            hashes.push_back(hashes.back());
        }
        // Combine hashes and hash the result
        std::string combinedData(hashes.size() * 32, '\0'); // Reserve space for combined hashes
        for (size_t i = 0; i < hashes.size(); i++) {
            memcpy(&combinedData[i * 32], hashes[i].begin(), 32);
        }
        std::string hash = SPHINXHash::SPHINX_256(combinedData);
        memcpy(hashes[0].begin(), hash.data(), 32);
        hashes.resize(hashes.size() / 2); // Reduce the number of hashes by half
    }
    // Set mutation flag if needed
    if (mutated) *mutated = mutation;
    // Return the computed Merkle root
    if (hashes.size() == 0) return uint256();
    return hashes[0];
}

// Compute the Merkle root of transactions in a block
uint256 BlockMerkleRoot(const CBlock& block, bool* mutated)
{
    std::vector<uint256> leaves;
    leaves.resize(block.vtx.size());
    // Get transaction hashes
    for (size_t s = 0; s < block.vtx.size(); s++) {
        leaves[s] = block.vtx[s]->GetHash();
    }
    // Compute Merkle root
    return ComputeMerkleRoot(std::move(leaves), mutated);
}

// Compute the Merkle root of transactions with witness data in a block
uint256 BlockWitnessMerkleRoot(const CBlock& block, bool* mutated)
{
    std::vector<uint256> leaves;
    leaves.resize(block.vtx.size());
    leaves[0].SetNull(); // The witness hash of the coinbase is 0.
    // Get witness transaction hashes
    for (size_t s = 1; s < block.vtx.size(); s++) {
        leaves[s] = block.vtx[s]->GetWitnessHash();
    }
    // Compute Merkle root
    return ComputeMerkleRoot(std::move(leaves), mutated);
}






