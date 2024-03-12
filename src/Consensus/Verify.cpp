// Copyright (c) [2023] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


// SPHINX Protocol Verification:
    // The verify_sphinx_protocol function creates instances of SPHINXProver and SPHINXVerifier to perform an interaction between them and verify the SPHINX protocol.
// Block Verification:
    // The verifyBlock function verifies the integrity of a block by recalculating its header hash and comparing it with the stored header hash. It then calls verify_sphinx_protocol to verify the SPHINX protocol.
// Chain Verification:
    // The verifyChain function iterates through all blocks in the chain, verifies each block's integrity, and checks if blocks are properly linked together. It also calls verify_sphinx_protocol to verify the SPHINX protocol.
// Transaction and Signature Verification:
    // Several functions are provided to verify transaction inputs, calculate sequence locks, evaluate sequence locks, count signature operations, and verify transaction signatures.
// Utility Functions:
    // Utility functions such as IsFinalTx, GetLegacySigOpCount, GetP2SHSigOpCount, etc., are provided for various tasks related to transaction and block validation.


#include <memory> // Include the <memory> header for std::unique_ptr
#include <utility> // Include the <utility> header for std::move

#include <chain.h>
#include <coins.h>

#include <consensus/validation.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <util/check.h>
#include <util/moneystr.h>

#include "Asset.hpp"
#include "Validation.hpp"
#include <Transaction.hpp>
#include "verify.hpp"
#include "Consensus.hpp"
#include "Crypto/Libstark/src/protocols/protocol.hpp"


namespace SPHINXVerify {

    // Function to verify the SPHINX protocol
    bool verify_sphinx_protocol() {
        // Create SPHINXProver and SPHINXVerifier objects
        SPHINXProver prover;
        SPHINXVerifier verifier;

        // Call sendMessage function from the verifier object to get the initial message
        auto initialMessage = verifier.sendMessage();

        // Create a unique_ptr and assign the initial message
        auto msg = std::make_unique<libstark::Protocols::TranscriptMessage>(std::move(initialMessage));

        // Continue the interaction until the verifier is done
        while (!verifier.doneInteracting()) {
            // Send the message to the prover
            prover.receiveMessage(*msg);

            // Call sendMessage function from the prover to get the response message
            auto responseMessage = prover.sendMessage();

            // Move the response message to the unique_ptr
            msg = std::make_unique<libstark::Protocols::TranscriptMessage>(std::move(responseMessage));

            // Send the message to the verifier
            verifier.receiveMessage(*msg);
        }

        // Verify the final result
        return verifier.verify();
    }

    // Function to verify the integrity of a block
    bool verifyBlock(const SPHINXBlock& block) {
        // Recalculate the hash of the block's header, including the Merkle root
        std::string calculatedHeaderHash = calculateBlockHeaderHash(block.getPreviousHash(), block.getMerkleRoot(), block.getTimestamp(), block.getNonce());

        // Compare the calculated hash with the Merkle root stored in the block's header
        if (calculatedHeaderHash == block.getHeaderHash()) {
            // If they match, the block's integrity is intact

            // Call the SPHINX protocol verification
            return verify_sphinx_protocol();
        } else {
            // Otherwise, the block has been tampered with
            return false;
        }
    }

    // Function to verify the integrity of the entire chain
    bool verifyChain(const SPHINXChain& chain) {
        size_t chainLength = chain.getChainLength();

        // An empty chain is considered valid
        if (chainLength == 0) {
            return verify_sphinx_protocol();
        }

        // Verify the integrity of each block in the chain
        for (size_t i = 0; i < chainLength; ++i) {
            const SPHINXBlock& currentBlock = chain.getBlockAt(i);

            // Verify the integrity of the current block
            if (!verifyBlock(currentBlock)) {
                // Invalid block detected
                return false;
            }

            if (i > 0) {
                const SPHINXBlock& previousBlock = chain.getBlockAt(i - 1);
                if (currentBlock.getPreviousHash() != previousBlock.getHash()) {
                    // The blocks are not properly linked together
                    return false;
                }
            }
        }

        // All blocks have been verified, and the chain is valid
        return true;
    }

    // Function to verify the integrity of a block and its signature
    bool verifySPHINXBlock(const SPHINXBlock& block, const std::string& signature, const SPHINXPubKey& publicKey) {
        // Verify the integrity of the block first (including SPHINX protocol verification)
        if (!verifyBlock(block)) {
            return false;
        }

        // Verify the signature of the block using the provided signature and public key
        return Crypto::verify(block.getBlockHash(), signature, publicKey);
    }

    // Function to verify the integrity of the entire chain, including block signatures
    bool verifySPHINXChain(const SPHINXChain& chain) {
        // Verify the integrity of the chain first (including SPHINX protocol verification)
        if (!verifyChain(chain)) {
            return false;
        }

        size_t chainLength = chain.getChainLength();

        // An empty chain is considered valid
        if (chainLength == 0) {
            return true;
        }

        // Verify the integrity of each block in the chain, including their signatures
        for (size_t i = 0; i < chainLength; ++i) {
            const SPHINXBlock& currentBlock = chain.getBlockAt(i);

            // Verify the signature of the current block
            if (!verifySPHINXBlock(currentBlock, currentBlock.getSignature(), currentBlock.getPublicKey())) {
                // Invalid block detected
                return false;
            }

            if (i > 0) {
                const SPHINXBlock& previousBlock = chain.getBlockAt(i - 1);
                if (currentBlock.getPreviousHash() != previousBlock.getHash()) {
                    // The blocks are not properly linked together
                    return false;
                }
            }
        }

        // All blocks have been verified, and the chain is valid
        return true;
    }


    // Function to verify the SPHINX protocol
    bool verify_sphinx_protocol();

    // Function to check if a transaction is final
    bool IsFinalTx(const SPHINXTrx::Transaction &tx, int nBlockHeight, int64_t nBlockTime) {
        // Check if the transaction's lock time is zero
        if (tx.nLockTime == 0) {
            return true;
        }

        // Check if the transaction's lock time is less than the block time or height
        if ((int64_t)tx.nLockTime < ((int64_t)tx.nLockTime < LOCKTIME_THRESHOLD ? (int64_t)nBlockHeight : nBlockTime)) {
            return true;
        }

        // Check if all inputs' sequence numbers are set to SEQUENCE_FINAL
        for (const auto& txin : tx.vin) {
            if (txin.nSequence != SPHINXTrx::CTxIn::SEQUENCE_FINAL) {
                return false;
            }
        }

        return true;
    }

    // Function to calculate sequence locks
    // The function iterates through each input of the transaction.
    // For each input, it calculates the minimum height based on the input's sequence number.
    // If the input's sequence number indicates a time-based lock, it calculates the minimum time based on the lock time specified in the sequence number.
    // The function returns the minimum height and minimum time as a pair.
    std::pair<int, int64_t> CalculateSequenceLocks(const SPHINXTrx::Transaction &tx, int flags, std::vector<int>& prevHeights, const CBlockIndex& block) {
        assert(prevHeights.size() == tx.vin.size());

        int nMinHeight = -1;
        int64_t nMinTime = -1;

        // Iterate through each input of the transaction
        for (size_t txinIndex = 0; txinIndex < tx.vin.size(); txinIndex++) {
            const SPHINXTrx::CTxIn& txin = tx.vin[txinIndex];
            int nCoinHeight = prevHeights[txinIndex];

            // Check if sequence locktime is disabled
            if (txin.nSequence & SPHINXTrx::CTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG) {
                // The height of this input is not relevant for sequence locks
                prevHeights[txinIndex] = 0;
                continue;
            }

            // Calculate minimum height based on input sequence
            nMinHeight = (nMinHeight == -1) ? (nCoinHeight + (int)(txin.nSequence & SPHINXTrx::CTxIn::SEQUENCE_LOCKTIME_MASK) - 1) : std::min(nMinHeight, nCoinHeight + (int)(txin.nSequence & SPHINXTrx::CTxIn::SEQUENCE_LOCKTIME_MASK) - 1);

            // Calculate minimum time based on input sequence
            if (txin.nSequence & SPHINXTrx::CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG) {
                int64_t nCoinTime = (nCoinHeight == 0) ? 0 : block.GetAncestor(nCoinHeight - 1)->GetMedianTimePast();
                int64_t sequenceTime = nCoinTime + (int64_t)((txin.nSequence & SPHINXTrx::CTxIn::SEQUENCE_LOCKTIME_MASK) << SPHINXTrx::CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) - 1;
                nMinTime = (nMinTime == -1) ? sequenceTime : std::min(nMinTime, sequenceTime);
            }
        }

        return std::make_pair(nMinHeight, nMinTime);
    }


    // Function to evaluate sequence locks
    bool EvaluateSequenceLocks(const CBlockIndex& block, std::pair<int, int64_t> lockPair) {
        assert(block.pprev);

        int64_t nBlockTime = block.pprev->GetMedianTimePast();

        if (lockPair.first >= block.nHeight || lockPair.second >= nBlockTime) {
            return false; // Sequence locks are not satisfied
        }

        return true; // Sequence locks are satisfied
    }

    // Function to perform sequence locks verification
    bool SequenceLocks(const SPHINXTrx::Transaction &tx, int flags, std::vector<int>& prevHeights, const CBlockIndex& block) {
        std::pair<int, int64_t> lockPair = CalculateSequenceLocks(tx, flags, prevHeights, block);
        return EvaluateSequenceLocks(block, lockPair);
    }

    // Function to get the number of legacy signature operations
    unsigned int GetLegacySigOpCount(const SPHINXTrx::Transaction& tx) {
        unsigned int nSigOps = 0;

        // Loop through the inputs and count the signature operations in their scriptSigs
        for (const auto& txin : tx.vin) {
            nSigOps += txin.scriptSig.GetSigOpCount(false);
        }

        // Loop through the outputs and count the signature operations in their scriptPubKeys
        for (const auto& txout : tx.vout) {
            nSigOps += txout.scriptPubKey.GetSigOpCount(false);
        }

        return nSigOps;
    }

    // Function to get the number of P2SH signature operations
    unsigned int GetP2SHSigOpCount(const SPHINXTrx::Transaction& tx, const CCoinsViewCache& inputs) {
        if (tx.IsCoinBase())
            return 0;

        unsigned int nSigOps = 0;

        for (const auto& txin : tx.vin) {
            const Coin& coin = inputs.AccessCoin(txin.prevout);
            assert(!coin.IsSpent());
            const CTxOut &prevout = coin.out;

            if (prevout.scriptPubKey.IsPayToScriptHash())
                nSigOps += prevout.scriptPubKey.GetSigOpCount(txin.scriptSig);
        }

        return nSigOps;
    }

    // Function to calculate the transaction signature operation cost
    int64_t GetTransactionSigOpCost(const SPHINXTrx::Transaction& tx, const CCoinsViewCache& inputs, uint32_t flags) {
        int64_t nSigOps = GetLegacySigOpCount(tx) * WITNESS_SCALE_FACTOR;

        if (tx.IsCoinBase())
            return nSigOps;

        if (flags & SCRIPT_VERIFY_P2SH) {
            nSigOps += GetP2SHSigOpCount(tx, inputs) * WITNESS_SCALE_FACTOR;
        }

        for (const auto& txin : tx.vin) {
            const Coin& coin = inputs.AccessCoin(txin.prevout);
            assert(!coin.IsSpent());
            const CTxOut &prevout = coin.out;

            nSigOps += CountWitnessSigOps(txin.scriptSig, prevout.scriptPubKey, &txin.scriptWitness, flags);
        }

        return nSigOps;
    }

    // Function to check transaction inputs
    bool CheckTxInputs(const SPHINXTrx::Transaction& tx, TxValidationState& state, const CCoinsViewCache& inputs, int nSpendHeight, CAmount& txfee) {
        // Validate the transaction inputs and perform necessary checks
        // You can access tx.data, tx.signature, tx.publicKey to perform checks

        // Check if the actual inputs are available in the inputs cache
        if (!inputs.HaveInputs(tx)) {
            return state.Invalid(TxValidationResult::TX_MISSING_INPUTS, "bad-txns-inputs-missingorspent",
                         strprintf("%s: inputs missing/spent", __func__));
        }

        CAmount nValueIn = 0;
        for (const auto& txin : tx.vin) {
            const Coin& coin = inputs.AccessCoin(txin.prevout);
            assert(!coin.IsSpent());

            // If prev is coinbase, check that it's matured
            if (coin.IsCoinBase() && nSpendHeight - coin.nHeight < SPHINXConsensus::COINBASE_MATURITY) {
                return state.Invalid(TxValidationResult::TX_PREMATURE_SPEND, "bad-txns-premature-spend-of-coinbase",
                    strprintf("tried to spend coinbase at depth %d", nSpendHeight - coin.nHeight));
            }

            // Check for negative or overflow input values
            nValueIn += coin.out.nValue;
            if (!MoneyRange(coin.out.nValue) || !MoneyRange(nValueIn)) {
                return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-inputvalues-outofrange");
            }
        }

        const CAmount value_out = tx.GetValueOut();
        if (nValueIn < value_out) {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-in-belowout",
                strprintf("value in (%s) < value out (%s)", FormatMoney(nValueIn), FormatMoney(value_out)));
        }

        // Calculate transaction fee
        const CAmount txfee_aux = nValueIn - value_out;
        if (!MoneyRange(txfee_aux)) {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-fee-outofrange");
        }

        txfee = txfee_aux;
        return true;
    }

} // namespace SPHINX_VERIFY