// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#ifndef MERKLEBLOCK_HPP
#define MERKLEBLOCK_HPP

#include <vector>
#include <string>
#include <set>  // For std::set

#include <common/bloom.hpp>
#include <primitives/block.hpp>
#include <serialize.hpp>
#include <uint256.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper functions for serialization.
std::vector<unsigned char> BitsToBytes(const std::vector<bool>& bits); // Convert vector of bits to bytes
std::vector<bool> BytesToBits(const std::vector<unsigned char>& bytes); // Convert vector of bytes to bits

class CPartialMerkleTree {
protected:
    unsigned int nTransactions; // Total number of transactions in the block
    std::vector<bool> vBits; // Node-is-parent-of-matched-txid bits
    std::vector<uint256> vHash; // Txids and internal hashes
    bool fBad; // Flag set when encountering invalid data

    unsigned int CalcTreeWidth(int height) const; // Helper function to efficiently calculate the number of nodes at given height in the merkle tree
    uint256 CalcHash(int height, unsigned int pos, const std::vector<uint256>& vTxid); // Calculate the hash of a node in the merkle tree
    void TraverseAndBuild(int height, unsigned int pos, const std::vector<uint256>& vTxid, const std::vector<bool>& vMatch); // Recursive function that traverses tree nodes, storing the data as bits and hashes
    uint256 TraverseAndExtract(int height, unsigned int pos, unsigned int& nBitsUsed, unsigned int& nHashUsed, std::vector<uint256>& vMatch, std::vector<unsigned int>& vnIndex); // Recursive function that traverses tree nodes, consuming the bits and hashes produced by TraverseAndBuild

public:
    // Convert object to JSON representation
    json toJson() const {
        return {
            {"nTransactions", nTransactions},
            {"vBits", vBits},
            {"vHash", vHash},
            {"fBad", fBad}
        };
    }

    // Construct a partial merkle tree from a list of transaction ids and a mask that selects a subset of them
    CPartialMerkleTree(const std::vector<uint256>& vTxid, const std::vector<bool>& vMatch);
    CPartialMerkleTree(); // Default constructor
    
    // Extract the matching txid's represented by this partial merkle tree and their respective indices within the partial tree
    // Returns the merkle root, or 0 in case of failure
    uint256 ExtractMatches(std::vector<uint256>& vMatch, std::vector<unsigned int>& vnIndex);
    // Get number of transactions the merkle proof is indicating for cross-reference with local blockchain knowledge
    unsigned int GetNumTransactions() const { return nTransactions; }
};

class CMerkleBlock {
public:
    CBlockHeader header;
    CPartialMerkleTree txn;
    std::vector<std::pair<unsigned int, uint256>> vMatchedTxn;

    // Convert object to JSON representation
    json toJson() const {
        return {
            {"header", header.toJson()},
            {"txn", txn.toJson()},
            {"vMatchedTxn", vMatchedTxn}
        };
    }

    // Create from a CBlock, filtering transactions according to filter
    // Note that this will call IsRelevantAndUpdate on the filter for each transaction, thus the filter will likely be modified
    CMerkleBlock(const CBlock& block, CBloomFilter& filter);
    // Create from a CBlock, matching the txids in the set
    CMerkleBlock(const CBlock& block, const std::set<uint256>& txids);
    CMerkleBlock(); // Default constructor

private:
    // Combined constructor to consolidate code
    CMerkleBlock(const CBlock& block, CBloomFilter* filter, const std::set<uint256>* txids);
};






