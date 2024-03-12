// Copyright (c) [2023] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#ifndef MERKLE_HPP
#define MERKLE_HPP

#include <vector>

#include <primitives/block.hpp>
#include <uint256.hpp>

uint256 ComputeMerkleRoot(std::vector<uint256> hashes, bool* mutated = nullptr);

/*
 * Compute the Merkle root of the transactions in a block.
 * *mutated is set to true if a duplicated subtree was found.
 */
uint256 BlockMerkleRoot(const CBlock& block, bool* mutated = nullptr);

/*
 * Compute the Merkle root of the witness transactions in a block.
 * *mutated is set to true if a duplicated subtree was found.
 */
uint256 BlockWitnessMerkleRoot(const CBlock& block, bool* mutated = nullptr);

#endif // MERKLE_HPP
