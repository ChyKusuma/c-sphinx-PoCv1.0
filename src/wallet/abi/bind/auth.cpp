#include <iostream>
#include <string>
#include <functional>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstdint>

#include <context.hpp>
#include <crypto/ecdsa.hpp>
#include <errors.hpp>
#include <math/big.hpp>
#include <accounts.hpp>
#include <accounts/external.hpp>
#include <accounts/keystore.hpp>
#include <common.hpp>
#include <core/types.hpp>
#include <crypto.hpp>
#include <log.hpp>

// Assuming the missing definitions and implementations of types like
// `TransactOpts`, `KeyStore`, `accounts::Account`, `ExternalSigner`, `common::Address`,
// `types::Transaction`, `crypto::PrivateKey`, etc. are available in the project.

namespace bind {
    // ErrNoChainID is returned whenever the user failed to specify a chain id.
    const std::runtime_error ErrNoChainID("no chain id specified");

    // ErrNotAuthorized is returned when an account is not properly unlocked.
    const std::runtime_error ErrNotAuthorized("not authorized to sign this account");

    // NewTransactor is a utility method to easily create a transaction signer from
    // an encrypted json key stream and the associated passphrase.
    // Deprecated: Use NewTransactorWithChainID instead.
    // This translation only supports the new version with chain ID.
    // If needed, it can be extended to support the deprecated version as well.
    // The dependent types like `TransactOpts` should be defined accordingly.
    // For now, it throws an exception for the deprecated method.
    TransactOpts* NewTransactor(std::istream& keyin, const std::string& passphrase) {
        throw std::runtime_error("WARNING: NewTransactor has been deprecated in favour of NewTransactorWithChainID");
    }

    // NewKeyStoreTransactor is a utility method to easily create a transaction signer from
    // an decrypted key from a keystore.
    // Deprecated: Use NewKeyStoreTransactorWithChainID instead.
    // Similar to NewTransactor, this translation only supports the new version with chain ID.
    // Throws an exception for the deprecated method.
    TransactOpts* NewKeyStoreTransactor(keystore::KeyStore& keystore, accounts::Account& account) {
        throw std::runtime_error("WARNING: NewKeyStoreTransactor has been deprecated in favour of NewTransactorWithChainID");
    }

    // NewKeyedTransactor is a utility method to easily create a transaction signer
    // from a single private key.
    // Deprecated: Use NewKeyedTransactorWithChainID instead.
    // Similar to NewTransactor, this translation only supports the new version with chain ID.
    // Throws an exception for the deprecated method.
    TransactOpts* NewKeyedTransactor(crypto::PrivateKey* key) {
        throw std::runtime_error("WARNING: NewKeyedTransactor has been deprecated in favour of NewKeyedTransactorWithChainID");
    }

    // NewTransactorWithChainID is a utility method to easily create a transaction signer from
    // an encrypted json key stream and the associated passphrase.
    TransactOpts* NewTransactorWithChainID(std::istream& keyin, const std::string& passphrase, const std::int64_t chainID) {
        std::string json;
        std::getline(keyin, json, '\0');
        crypto::PrivateKey* key = keystore::DecryptKey(json, passphrase);
        return NewKeyedTransactorWithChainID(key, chainID);
    }

    // NewKeyStoreTransactorWithChainID is a utility method to easily create a transaction signer from
    // an decrypted key from a keystore.
    TransactOpts* NewKeyStoreTransactorWithChainID(keystore::KeyStore& keystore, accounts::Account& account, const std::int64_t chainID) {
        if (chainID == 0) {
            throw ErrNoChainID;
        }
        types::SignerFn signer = types::LatestSignerForChainID(chainID);
        return new TransactOpts{account.Address, [&keystore, account, signer](common::Address address, types::Transaction* tx) {
            if (address != account.Address) {
                throw ErrNotAuthorized;
            }
            auto signature = keystore.SignHash(account, signer.Hash(tx).Bytes());
            return tx->WithSignature(signer, signature);
        }, context::background()};
    }

    // NewKeyedTransactorWithChainID is a utility method to easily create a transaction signer
    // from a single private key.
    TransactOpts* NewKeyedTransactorWithChainID(crypto::PrivateKey* key, const std::int64_t chainID) {
        common::Address keyAddr = crypto::PubkeyToAddress(key->PublicKey);
        if (chainID == 0) {
            throw ErrNoChainID;
        }
        types::SignerFn signer = types::LatestSignerForChainID(chainID);
        return new TransactOpts{keyAddr, [&key, keyAddr, signer](common::Address address, types::Transaction* tx) {
            if (address != keyAddr) {
                throw ErrNotAuthorized;
            }
            auto signature = crypto.Sign(signer.Hash(tx).Bytes(), key);
            return tx->WithSignature(signer, signature);
        }, context::background()};
    }

    // NewClefTransactor is a utility method to easily create a transaction signer
    // with a clef backend.
    TransactOpts* NewClefTransactor(external::ExternalSigner* clef, accounts::Account& account) {
        return new TransactOpts{account.Address, [&clef, account](common::Address address, types::Transaction* transaction) {
            if (address != account.Address) {
                throw ErrNotAuthorized;
            }
            return clef->SignTx(account, transaction, nullptr); // Clef enforces its own chain id
        }, context::background()};
    }
}