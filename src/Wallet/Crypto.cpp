// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#include <iostream>
#include <vector>
#include <sstream> // for std::ostringstream
#include <iomanip> // for std::setw, std::hex, and std::setfill
#include <crypto/openssl/evp.h> // for all other OpenSSL function calls
#include <crypto/openssl/rand.h> // for RAND_bytes
#include <crypto/openssl/sha.h> // for SHA3-512_DIGEST_LENGTH
#include <crypto/openssl/aes.h> // for AES encryption

#define WALLET_CRYPTO_KEY_SIZE 32
#define WALLET_CRYPTO_IV_SIZE 16
#define AES_BLOCKSIZE 16

namespace SPHINXWallet {

    // AES256 CBC Encryptor
    class AES256CBCEncrypt {
    public:
        // Constructor
        AES256CBCEncrypt(const unsigned char* key, const unsigned char* iv, bool padding) {
            EVP_CIPHER_CTX_init(&ctx_);
            EVP_EncryptInit_ex(&ctx_, EVP_aes_256_cbc(), nullptr, key, iv);
            EVP_CIPHER_CTX_set_padding(&ctx_, padding ? 1 : 0);
        }

        // Destructor
        ~AES256CBCEncrypt() {
            EVP_CIPHER_CTX_cleanup(&ctx_);
        }

        // Encryption function
        size_t Encrypt(const unsigned char* plaintext, size_t plaintext_len, unsigned char* ciphertext) {
            int len;
            size_t ciphertext_len;

            EVP_EncryptUpdate(&ctx_, ciphertext, &len, plaintext, plaintext_len);
            ciphertext_len = len;

            EVP_EncryptFinal_ex(&ctx_, ciphertext + len, &len);
            ciphertext_len += len;

            return ciphertext_len;
        }

    private:
        EVP_CIPHER_CTX ctx_;
    };

    // SHA3-512 Hash function
    std::string sha3_512(const std::string& input)
    {
        uint32_t digest_length = SHA3_512_DIGEST_LENGTH;
        const EVP_MD* algorithm = EVP_sha3_512();
        uint8_t* digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
        EVP_MD_CTX* context = EVP_MD_CTX_new();
        
        // Initialize the context
        EVP_DigestInit_ex(context, algorithm, nullptr);
        
        // Update the context with input data
        EVP_DigestUpdate(context, input.c_str(), input.size());
        
        // Finalize the context and obtain the digest
        EVP_DigestFinal_ex(context, digest, &digest_length);
        
        // Clean up the context
        EVP_MD_CTX_destroy(context);
        
        // Convert digest to hex string
        std::string output = bytes_to_hex_string(std::vector<uint8_t>(digest, digest + digest_length));
        
        // Free the allocated memory
        OPENSSL_free(digest);
        
        return output;
    }

    // Helper function to print the digest bytes as a hex string
    std::string bytes_to_hex_string(const std::vector<uint8_t>& bytes) {
        std::ostringstream stream;
        for (uint8_t b : bytes) {
            stream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(b);
        }
        return stream.str();
    }

    // Function to derive AES key and IV from passphrase using SHA3-512
    int BytesToKeySHA3AES(const std::vector<unsigned char>& chSalt, const std::string& strKeyData, int count, unsigned char* key, unsigned char* iv) {
        if (!count || !key || !iv)
            return 0;

        unsigned char buf[SHA3_512_DIGEST_LENGTH];
        SHA3_CTX ctx;
        SHA3_Init(&ctx);
        SHA3_Update(&ctx, reinterpret_cast<const unsigned char*>(strKeyData.c_str()), strKeyData.size());
        SHA3_Update(&ctx, chSalt.data(), chSalt.size());
        SHA3_Final(buf, &ctx);

        for (int i = 0; i != count - 1; i++) {
            SHA3_Init(&ctx);
            SHA3_Update(&ctx, buf, sizeof(buf));
            SHA3_Final(buf, &ctx);
        }

        memcpy(key, buf, WALLET_CRYPTO_KEY_SIZE);
        memcpy(iv, buf + WALLET_CRYPTO_KEY_SIZE, WALLET_CRYPTO_IV_SIZE);

        return WALLET_CRYPTO_KEY_SIZE;
    }

    // Crypter class for encryption and decryption
    class CCrypter {
    public:
        // Function to set key from passphrase
        bool SetKeyFromPassphrase(const std::string& strKeyData, const std::vector<unsigned char>& chSalt, const unsigned int nRounds, const unsigned int nDerivationMethod) {
            if (nRounds < 1 || chSalt.size() != WALLET_CRYPTO_SALT_SIZE)
                return false;

            int i = 0;
            if (nDerivationMethod == 0)
                i = BytesToKeySHA3AES(chSalt, strKeyData, nRounds, vchKey.data(), vchIV.data());

            if (i != (int)WALLET_CRYPTO_KEY_SIZE) {
                memory_cleanse(vchKey.data(), vchKey.size());
                memory_cleanse(vchIV.data(), vchIV.size());
                return false;
            }

            fKeySet = true;
            return true;
        }

        // Function to set key directly
        bool SetKey(const std::vector<unsigned char>& chNewKey, const std::vector<unsigned char>& chNewIV) {
            if (chNewKey.size() != WALLET_CRYPTO_KEY_SIZE || chNewIV.size() != WALLET_CRYPTO_IV_SIZE)
                return false;

            memcpy(vchKey.data(), chNewKey.data(), chNewKey.size());
            memcpy(vchIV.data(), chNewIV.data(), chNewIV.size());

            fKeySet = true;
            return true;
        }

        // Function to encrypt data
        bool Encrypt(const std::vector<unsigned char>& vchPlaintext, std::vector<unsigned char>& vchCiphertext) const {
            if (!fKeySet)
                return false;

            // Max ciphertext len for a n bytes of plaintext is n + AES_BLOCKSIZE bytes
            vchCiphertext.resize(vchPlaintext.size() + AES_BLOCKSIZE);

            AES256CBCEncrypt enc(vchKey.data(), vchIV.data(), true);
            size_t nLen = enc.Encrypt(vchPlaintext.data(), vchPlaintext.size(), vchCiphertext.data());
            if (nLen < vchPlaintext.size())
                return false;
            vchCiphertext.resize(nLen);

            return true;
        }

        // Function to decrypt data
        bool Decrypt(const std::vector<unsigned char>& vchCiphertext, std::vector<unsigned char>& vchPlaintext) const {
            if (!fKeySet)
                return false;

            // Plaintext will always be equal to or lesser than the length of ciphertext
            int nLen = vchCiphertext.size();
            vchPlaintext.resize(nLen);

            AES256CBCDecrypt dec(vchKey.data(), vchIV.data(), true);
            nLen = dec.Decrypt(vchCiphertext.data(), vchCiphertext.size(), vchPlaintext.data());
            if (nLen == 0)
                return false;
            vchPlaintext.resize(nLen);
            return true;
        }

    private:
        bool fKeySet = false;
        std::vector<unsigned char> vchKey = std::vector<unsigned char>(WALLET_CRYPTO_KEY_SIZE);
        std::vector<unsigned char> vchIV = std::vector<unsigned char>(WALLET_CRYPTO_IV_SIZE);
    };
} // namespace SPHINXWallet
