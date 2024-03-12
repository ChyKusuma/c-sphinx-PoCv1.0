// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#include <iostream>
#include <ctime>
#include <string>

#include "Block.hpp"
#include <Hash.hpp>
#include <SphinxJS/jsonrpcpp/include/json.hpp>

namespace SPHINXBlock {

    // Structure representing a block header
    struct BlockHeader {
        // Block header fields
        uint32_t nVersion;
        std::string hashPrevBlock;
        std::string hashMerkleRoot;
        std::string signature;
        uint32_t blockHeight;
        std::time_t nTimestamp;
        uint32_t nNonce;
        uint32_t nDifficulty;

        // Function to calculate the hash of the block header
        SPHINXHash::SPHINX_256 GetHash() const {
            // Create a JSON representation of the object
            nlohmann::json jsonRepresentation = {
                {"version", nVersion},
                {"hashPrevBlock", hashPrevBlock},
                {"hashMerkleRoot", hashMerkleRoot},
                {"signature", signature},
                {"blockHeight", blockHeight},
                {"timestamp", nTimestamp},
                {"nonce", nNonce},
                {"difficulty", nDifficulty}
            };

            // Convert the JSON to a string
            std::string jsonString = jsonRepresentation.dump();

            // Calculate the hash using SPHINX_256 hash function
            return SPHINXHash::Hash(jsonString); // Assuming Hash is the function to calculate the hash
        }
    }; // End of BlockHeader struct

    // Function to convert BlockHeader to string
    std::string BlockHeader::ToString() const {
        // Create a string stream to concatenate information
        std::stringstream ss;
        // Append block information to the string stream
        ss << "Block:\n";
        ss << "  Hash: " << GetHash() << "\n";
        ss << "  Version: 0x" << std::hex << nVersion << "\n"; // Output block version in hexadecimal format
        ss << "  Prev Block: " << hashPrevBlock << "\n"; // Output previous block hash
        ss << "  Merkle Root: " << hashMerkleRoot << "\n"; // Output Merkle root hash
        ss << "  Time: " << nTimestamp << "\n"; // Output block timestamp
        ss << "  Nonce: " << nNonce << "\n"; // Output block nonce
        ss << "  Difficulty: " << nDifficulty << "\n"; // Output block difficulty

        // Output number of transactions in the block
        ss << "  Transactions: " << transactions.size() << "\n"; // Assuming transactions_ holds transaction data

        // Output each transaction in the block
        for (const auto& tx : transactions) {
            ss << "  Transaction: " << tx << "\n";
        }

        // Return the concatenated string
        return ss.str();
    }
} // namespace SPHINXBlock
