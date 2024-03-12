// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#ifndef SPHINX_WALLET_CRYPTER_H
#define SPHINX_WALLET_CRYPTER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include <serialize.h>
#include <support/allocators/secure.h>
#include <script/signingprovider.h>

// Constants for key, salt, and IV sizes
namespace SPHINXWallet {
    // Sizes of cryptographic elements
    const unsigned int WALLET_CRYPTO_KEY_SIZE = 32;
    const unsigned int WALLET_CRYPTO_SALT_SIZE = 8;
    const unsigned int WALLET_CRYPTO_IV_SIZE = 16;

    /**
     * Private key encryption is done based on a CMasterKey,
     * which holds a salt and random encryption key.
     *
     * CMasterKeys are encrypted using AES-256-CBC using a key
     * derived using derivation method nDerivationMethod
     * (0 == EVP_SHA3_512()) and derivation iterations nDeriveIterations.
     * vchOtherDerivationParameters is provided for alternative algorithms
     * which may require more parameters (such as scrypt).
     *
     * Wallet Private Keys are then encrypted using AES-256-CBC
     * with the double SPHINX_256 of the public key as the IV, and the
     * master key's key as the encryption key (see keystore.[ch]).
     */

    /** Master key for wallet encryption */
    class CMasterKey
    {
    public:
        std::vector<unsigned char> vchCryptedKey;
        std::vector<unsigned char> vchSalt;
        //! 0 = EVP_SHA3_512()
        //! 1 = scrypt()
        unsigned int nDerivationMethod;
        unsigned int nDeriveIterations;
        //! Use this for more parameters to key derivation,
        //! such as the various parameters to scrypt
        std::vector<unsigned char> vchOtherDerivationParameters;

        SERIALIZE_METHODS(CMasterKey, obj)
        {
            READWRITE(obj.vchCryptedKey, obj.vchSalt, obj.nDerivationMethod, obj.nDeriveIterations, obj.vchOtherDerivationParameters);
        }

        CMasterKey()
        {
            // 25000 rounds is just under 0.1 seconds on a 1.86 GHz Pentium M
            // ie slightly lower than the lowest hardware we need bother supporting
            nDeriveIterations = 25000;
            nDerivationMethod = 0;
            vchOtherDerivationParameters = std::vector<unsigned char>(0);
        }
    };

    // Alias for secure vector
    typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;

    namespace wallet_crypto_tests
    {
        class TestCrypter;
    }

    /** Encryption/decryption context with key information */
    class CCrypter
    {
    friend class wallet_crypto_tests::TestCrypter; // for test access to chKey/chIV
    private:
        std::vector<unsigned char, secure_allocator<unsigned char>> vchKey;
        std::vector<unsigned char, secure_allocator<unsigned char>> vchIV;
        bool fKeySet;

        // Helper function to derive key from passphrase using SHA3 and AES
        int BytesToKeySHA3AES(const std::vector<unsigned char>& chSalt, const SecureString& strKeyData, int count, unsigned char *key,unsigned char *iv) const;

    public:
        // Set key from passphrase
        bool SetKeyFromPassphrase(const SecureString &strKeyData, const std::vector<unsigned char>& chSalt, const unsigned int nRounds, const unsigned int nDerivationMethod);
        // Encrypt data
        bool Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext) const;
        // Decrypt data
        bool Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext) const;
        // Set encryption key
        bool SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV);

        // Clean up key
        void CleanKey()
        {
            memory_cleanse(vchKey.data(), vchKey.size());
            memory_cleanse(vchIV.data(), vchIV.size());
            fKeySet = false;
        }

        // Constructor
        CCrypter()
        {
            fKeySet = false;
            vchKey.resize(WALLET_CRYPTO_KEY_SIZE);
            vchIV.resize(WALLET_CRYPTO_IV_SIZE);
        }

        // Destructor
        ~CCrypter()
        {
            CleanKey();
        }
    };

    // Encrypt data using secret
    bool EncryptSecret(const CKeyingMaterial& vMasterKey, const CKeyingMaterial &vchPlaintext, const SPHINXHash::SPHINX_256& nIV, std::vector<unsigned char> &vchCiphertext);
    // Decrypt data using secret
    bool DecryptSecret(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCiphertext, const SPHINXHash::SPHINX_256& nIV, CKeyingMaterial& vchPlaintext);
    // Decrypt key using secret
    bool DecryptKey(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCryptedSecret, const CPubKey& vchPubKey, CKey& key);
} // namespace SPHINXWallet



