// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#ifndef SPHINX_CONSENSUS_HPP
#define SPHINX_CONSENSUS_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <stdint.h>

namespace SPHINXConsensus {

     // Forward declaration of Transaction
    struct Transaction;

    // Define the block structure
    struct Block {
        uint32_t nVersion;
        std::string hashPrevBlock;
        std::string hashMerkleRoot;
        std::string signature;
        uint32_t blockHeight;
        std::time_t nTimestamp;
        uint32_t nNonce;
        uint64_t nDifficulty;
        uint64_t blockReward;
        std::string minerAddress;
        uint32_t blockNonce;
        // Other block-related data
    };

    // Define the transaction structure
    struct Transaction {
        // Transaction data
    };

    // Define the maximum block size
    static const uint64_t MAX_BLOCK_SIZE = 4000000;

    // Define the gas limit for transactions
    static const uint64_t GAS_LIMIT = 1000000;

    // Define the maximum number of transactions in a block
    static const uint32_t MAX_TRANSACTIONS_PER_BLOCK = 1000;

    // Define the coinbase maturity
    static const uint64_t COINBASE_MATURITY = 100;

    // Define the consensus mechanism's weight factor
    static const uint32_t WITNESS_SCALE_FACTOR = 4;

    // Define the minimum transaction weight
    static const uint64_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60;

    // Define the minimum serializable transaction weight
    static const uint64_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10;

    // Define consensus flags and options
    enum ConsensusFlags {
        LOCKTIME_VERIFY_SEQUENCE = (1 << 0),
        // Other consensus flags
    };

    // Define functions to validate transactions and blocks
    bool ValidateTransaction(const Transaction& tx);
    bool ValidateBlock(const Block& block);

} // namespace SPHINXConsensus

#endif /* SPHINX_CONSENSUS_HPP */

