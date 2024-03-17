// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <random>
#include "Wallet.hpp"
#include "Transaction.hpp" 
#include "Key.hpp"
#include "Consensus/Contract.hpp"
#include "db.hpp"
#include "json.hpp"


using json = nlohmann::json;

namespace SPHINXWallet {

    // Define the WalletInfo struct inside the SPHINXWallet namespace
    struct WalletInfo {
        std::string address;
        std::string privateKey;
        std::string encryptedPassphrase;
        double balance;
        std::vector<SPHINXTrx::Transaction> transactions;
    };

    Wallet::Wallet() {
        // Initialize member variables
        walletAddress_ = "";
        privateKey_ = "";
        encryptedPassphrase_ = "";
        balance_ = 0.0;
        transactions_ = std::vector<SPHINXTrx::Transaction>();

        // Check if the wallet address is already in use.
        if (isWalletAddressInUse(walletAddress_)) {
            throw std::invalid_argument("Wallet address is already in use");
        }

        // Check if the passphrase is valid. 
        if (!isValidPassphrase(encryptedPassphrase_)) {
            throw std::invalid_argument("Passphrase is invalid");
        }

        // Initialize the transaction history.
        transactions_ = loadTransactionHistory();

        // Initialize the balance.
        balance_ = loadBalance();
    }

    bool Wallet::performIdentityVerification() {
        bool isMobile = detectMobileDevice(); // Replace with actual detection logic

        if (isMobile) {
            bool biometricSuccess = captureBiometricSample();
            return biometricSuccess;
        } else {
            bool passwordSuccess = verifyPassword();
            return passwordSuccess;
        }
    }

    bool Wallet::verifyPassword() {
        std::string enteredPassword;
        std::cout << "Enter password: ";
        std::cin >> enteredPassword;

        // Replace this with actual password verification logic
        bool isValidPassword = validatePassword(enteredPassword);

        return isValidPassword;
    }

    // Example Android Fingerprint API (Android-specific)
    bool Wallet::captureBiometricSample() {
        // Use Android Fingerprint API to capture fingerprint data
        // ...
        return true;
    }

    // Example iOS Local Authentication framework (iOS-specific)
    bool Wallet::captureBiometricSample() {
        // Use Local Authentication framework to capture fingerprint data
        // ...
        return true;
    }

    bool Wallet::performBiometricVerification() {
        // Capture biometric sample using the biometric sensor
        std::string capturedBiometricData = captureBiometricSample();

        // Retrieve the stored biometric template
        std::string storedBiometricTemplate = loadBiometricTemplate();

        // Perform template matching and threshold check
        double matchScore = calculateMatchScore(capturedBiometricData, storedBiometricTemplate);
        if (matchScore >= biometricThreshold) {
            return true; // Biometric verification successful
        } else {
            return false; // Biometric verification failed
        }
    }

    void Wallet::generateAccount() {
        bool isVerified = performIdentityVerification();
        if (!isVerified) {
            std::cout << "Identity verification failed. Account generation aborted." << std::endl;
            return;
        }
        // Prompt the user to enter a passphrase
        std::cout << "Enter passphrase: ";
        std::cin >> passphrase_;

        // Perform biometric verification
        if (!performBiometricVerification()) {
            std::cout << "Biometric verification failed. Account generation aborted." << std::endl;
            return;
        }

        // Generate a new wallet address and private key
        walletAddress_ = generateWalletAddress();
        privateKey_ = generatePrivateKey();

        // Perform key exchange using the exposed function from key.cpp
        SPHINXHybridKey::HybridKeypair hybridKeyPair = generate_and_perform_key_exchange();

        // Save the wallet information
        saveWalletInfo(walletAddress_, privateKey_, encryptedPassphrase);

        std::cout << "Account generated successfully!" << std::endl;
    }

    void Wallet::getAccountBalance() {
        // Retrieve the account balance from the blockchain
        balance_ = fetchAccountBalance(walletAddress_);

        std::cout << "Account Balance: " << balance_ << std::endl;
    }

    void Wallet::sendTransaction(const std::string& recipientAddress, double amount) {
        // Perform necessary operations to create and send the transaction.
        SPHINXContract::Transaction transaction;
        transaction.senderAddress = walletAddress_;
        transaction.recipientAddress = recipientAddress;
        transaction.amount = amount;
        transaction.timestamp = std::time(nullptr);
        transaction.sign(privateKey_);
        transaction.senderPublicKey = getPublicKey();

        // Use decryptedPassphrase to temporarily unlock the private key
        transaction.sign(decryptedPassphrase);

        // Add the transaction to the contract for processing
        SPHINXContract::processTransaction(transaction.toJson());

        std::cout << "Transaction sent successfully!" << std::endl;
    }

    void Wallet::getTransactionHistory() {
        // Retrieve the transaction history from the blockchain
        transactions_ = fetchTransactionHistory(walletAddress_);

        // Display the transaction history
        std::cout << "Transaction History:" << std::endl;
        for (const auto& transaction : transactions_) {
            std::cout << "Timestamp: " << transaction.timestamp << std::endl;
            std::cout << "Sender: " << transaction.sender << std::endl;
            std::cout << "Recipient: " << transaction.recipient << std::endl;
            std::cout << "Amount: " << transaction.amount << std::endl;
            std::cout << std::endl;
        }
    }

    void Wallet::createToken(const std::string& tokenName, const std::string& tokenSymbol) {
        // Create an instance of the SPHINXContract smart contract
        SPHINXContract tokenContract(tokenContractAddress);

        // Call the createToken function on the smart contract
        tokenContract.createToken(tokenName, tokenSymbol);

        std::cout << "Token created successfully!" << std::endl;
    }

    void Wallet::transferToken(const std::string& recipientAddress, const std::string& tokenSymbol, double amount) {
        // Create an instance of the SPHINXContract smart contract
        SPHINXContract tokenContract(tokenContractAddress);

        // Call the transfer function on the smart contract
        tokenContract.transfer(recipientAddress, tokenSymbol, amount);

        std::cout << "Tokens transferred successfully!" << std::endl;
    }

    void Wallet::interactWithSmartContract(const std::string& contractAddress, const std::string& functionName, const std::vector<std::string>& parameters) {
        // Create an instance of the SPHINXContract smart contract
        SPHINXContract contract(contractAddress);

        // Perform the desired interaction with the smart contract based on the function name and parameters
        if (functionName == "function1") {
            // Example interaction with function1
            std::string result = contract.function1(parameters[0]);
            std::cout << "Function1 result: " << result << std::endl;
        } else if (functionName == "function2") {
            // Example interaction with function2
            std::string result = contract.function2(parameters[0], parameters[1]);
            std::cout << "Function2 result: " << result << std::endl;
        } else {
            std::cout << "Invalid function name!" << std::endl;
        }
    }

    void Wallet::createWallet() {
        // Generate a new wallet address and private key
        walletAddress_ = generateWalletAddress();
        privateKey_ = generatePrivateKey();

        // Generate a new smart contract address based on the wallet's public key
        std::string contractAddress = generateAddress(getPublicKey(), "MyWalletContract");

        // Prompt the user to enter a passphrase
        std::cout << "Enter passphrase: ";
        std::cin >> passphrase_;

        // Encrypt the passphrase
        std::string encryptedPassphrase = encryptPassphrase(passphrase_);

        // Save the wallet information
        saveWalletInfo(walletAddress_, privateKey_, encryptedPassphrase);

        std::cout << "Wallet created successfully!" << std::endl;
        std::cout << "Smart Contract Address: " << contractAddress << std::endl;
    }

    void Wallet::initiateTransaction(const std::string& recipientAddress, double amount) {
        // Implementation of initiating a transaction...
        // Use privateKey_ and other member variables as needed...
        // Perform necessary operations to create and send the transaction.
        // Example code:
        SPHINXTrx::Transaction transaction;
        transaction.addInput(walletAddress_);
        transaction.addOutput(recipientAddress, amount);
        transaction.signTransaction(privateKey_);
        transaction.sendTransaction();

        // Use decryptedPassphrase to temporarily unlock the private key
        transaction.signTransaction(decryptedPassphrase);
    }

    // New function to request decryption from key component
    std::string Wallet::requestDecryption(const std::string& encryptedData) {
        // Communicate with key component to decrypt the data
        return SPHINXKey::decryptData(encryptedData);
    }

    // Function to generate a smart contract address
    std::string Wallet::generateSmartContractAddress(const std::string& publicKey, const std::string& contractName) {
        return SPHINXKey::generateAddress(publicKey, contractName);
    }

    std::string Wallet::generateRandomWord(std::vector<std::string>& wordList) {
        // Check if the word list is empty
        if (wordList.empty()) {
            // Return an empty string if the word list is empty
            return "";
        }

        // Initialize a random number generator
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, wordList.size() - 1);

        // Generate a random index to select a word from the list
        int index = dis(gen);

        // Get the randomly generated word
        std::string word = wordList[index];

        // Remove the selected word from the list
        wordList.erase(wordList.begin() + index);

        // Return the randomly generated word
        return word;
    }

    // Function to generate wallet address
    std::string Wallet::generateWalletAddress() {
        SPHINXKey sphinxKey; // Create an instance of the SPHINXKey class
        std::string publicKey = sphinxKey.getPublicKey(); // Get the wallet's public key
        std::string contractName = "MyWalletContract"; // Set the contract name as per your requirements

        return sphinxKey.generateAddress(publicKey, contractName); // Generate the wallet address
    }

    std::string Wallet::generatePrivateKey() {
        SPHINXKey sphinxKey; // Create an instance of the SPHINXKey class
        return sphinxKey.generatePrivateKey(); // Generate the private key using the SPHINXKey class
    }

    void Wallet::saveWalletInfo(const std::string& walletAddress, const std::string& privateKey, const std::string& encryptedPassphrase) {
        // Create a connection to SPHINXDb
        SPHINXDb::DistributedDb distributedDb;
        distributedDb.addNode("node1");
        distributedDb.addNode("node2");

        // Prepare the wallet data for saving
        SPHINXDb::Data walletData;
        walletData["address"] = walletAddress;
        walletData["privateKey"] = privateKey;
        walletData["encryptedPassphrase"] = encryptedPassphrase;

        // Serialize the wallet data
        std::string serializedData = serializeData(walletData);

        // Store the wallet data in the distributed database
        distributedDb.storeTransaction(walletAddress, serializedData);

        std::cout << "Wallet information saved successfully!" << std::endl;
    }

    void Wallet::loadWalletInfo() {
        // Perform biometric verification
        if (!performBiometricVerification()) {
            std::cout << "Biometric verification failed. Wallet information loading aborted." << std::endl;
            return;
        }

        // Create a connection to SPHINXDb
        SPHINXDb::DistributedDb distributedDb;
        distributedDb.addNode("node1");
        distributedDb.addNode("node2");

        // Retrieve the serialized wallet data from the distributed database based on the wallet address
        std::string serializedData = distributedDb.getTransactionData(walletAddress_);

        if (serializedData.empty()) {
            std::cout << "Wallet information not found." << std::endl;
            return;
        }
        
        // Decrypt the encrypted passphrase using the key functions from key.cpp
        std::string decryptedPassphrase = requestDecryption(walletInfo_.encryptedPassphrase);

        // Deserialize the wallet data
        SPHINXDb::Data walletData = deserializeData(serializedData);

        // Retrieve the wallet information from the deserialized data
        walletInfo_.address = walletData["address"].toString();
        walletInfo_.privateKey = walletData["privateKey"].toString();
        walletInfo_.encryptedPassphrase = walletData["encryptedPassphrase"].toString();

        std::cout << "Wallet information loaded successfully!" << std::endl;
    }

    std::string Wallet::serializeData(const SPHINXDb::Data& data) {
        // Serialize the data using nlohmann JSON library
        nlohmann::json jsonData = data;

        // Convert the JSON data to a string
        std::string serializedData = jsonData.dump();

        return serializedData;
    }

    SPHINXDb::Data Wallet::deserializeData(const std::string& serializedData) {
        // Parse the serialized data from string to JSON
        nlohmann::json jsonData = nlohmann::json::parse(serializedData);

        // Convert the JSON data to SPHINXDb::Data
        SPHINXDb::Data data = jsonData.get<SPHINXDb::Data>();

        return data;
    }

    std::string Wallet::encryptPassphrase(const std::string& passphrase) {
    // Replace this with your actual encryption logic using a cryptographic library
    std::string encryptedPassphrase = SPHINXKey::encryptData(passphrase);

        return encryptedPassphrase; // Return the encrypted passphrase
    }

    std::string Wallet::decryptPassphrase(const std::string& encryptedPassphrase) {
        // Replace this with your actual decryption logic using a cryptographic library
        std::string decryptedPassphrase = SPHINXKey::decryptData(encryptedPassphrase);

        return decryptedPassphrase; // Return the decrypted passphrase
    }

    double Wallet::fetchAccountBalance(const std::string& address) {
        // Replace this with actual logic to fetch the account balance from the blockchain
        // ...
        double accountBalance = 0.0; // Sample account balance for illustration
        return accountBalance;
    }

    std::vector<SPHINXTrx::Transaction> Wallet::fetchTransactionHistory(const std::string& address) {
        // Replace this with actual logic to fetch the transaction history from the blockchain
        // ...
        std::vector<SPHINXTrx::Transaction> transactions; // Sample transactions for illustration
        return transactions;
    }

    bool Wallet::isValidPassphrase(const std::string& passphrase) {
        // Replace this with actual logic to validate the passphrase
        // ...
        bool isValid = true; // Sample passphrase validation for illustration
        return isValid;
    }

    bool Wallet::isWalletAddressInUse(const std::string& address) {
        // Replace this with actual logic to check if the wallet address is already in use
        // ...
        bool isInUse = false; // Sample address check for illustration
        return isInUse;
    }

    double Wallet::loadBalance() {
        // Replace this with actual logic to load the balance from the appropriate data source
        // ...
        double loadedBalance = 0.0; // Sample loaded balance for illustration
        return loadedBalance;
    }

    std::vector<SPHINXTrx::Transaction> Wallet::loadTransactionHistory() {
        // Replace this with actual logic to load the transaction history from the appropriate data source
        // ...
        std::vector<SPHINXTrx::Transaction> transactions; // Sample transactions for illustration
        return transactions;
    }

    std::string Wallet::getPublicKey() {
        // Replace this with actual logic to retrieve the wallet's public key
        // ...
        std::string publicKey = "sample_public_key"; // Sample public key for illustration
        return publicKey;
    }
} // namespace SPHINXWallet
