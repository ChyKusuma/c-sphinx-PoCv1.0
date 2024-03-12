// Copyright (c) [2023-2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.


#include <cassert>
#include <vector>
#include <string>
#include <limits>
#include <cstring>
#include <algorithm>
#include <cstdint>

#include <base58.hpp>

#include <hash.hpp>
#include <uint256.hpp>
#include <util/strencodings.h>
#include <util/string.h>

#include <assert.h>
#include <string.h>

#include <limits>
#include <utilstrencodings.h> // Contains IsSpace, ContainsNoNUL, and Span


// All alphanumeric characters except for "0", "I", "O", and "l"
constexpr char Base58Characters[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// Map of base58 characters to their integer values
constexpr int8_t Base58Map[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
};

// Decode a Base58-encoded string into a vector of bytes
[[nodiscard]] static bool DecodeBase58(const char* str, std::vector<unsigned char>& result, int maxRetLength) {
    while (*str && IsSpace(*str)) // Skip leading spaces
        str++;

    int zeroes = 0;
    while (*str == '1') { // Skip leading '1's
        zeroes++;
        if (zeroes > maxRetLength)
            return false;
        str++;
    }

    int size = std::strlen(str) * 733 / 1000 + 1;
    std::vector<unsigned char> b256(size);

    while (*str && !IsSpace(*str)) {
        int carry = Base58Map[static_cast<uint8_t>(*str)];
        if (carry == -1)
            return false;

        int i = 0;
        for (auto it = b256.rbegin(); (carry != 0 || i < size) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        size = i;

        if (size + zeroes > maxRetLength)
            return false;
        str++;
    }

    while (IsSpace(*str)) // Skip trailing spaces
        str++;
    if (*str != 0)
        return false;

    auto it = b256.begin() + (size - zeroes);
    result.reserve(zeroes + (b256.end() - it));
    result.assign(zeroes, 0x00);
    while (it != b256.end())
        result.push_back(*(it++));
    return true;
}

// Encode a vector of bytes into a Base58-encoded string
std::string EncodeBase58(Span<const unsigned char> input) {
    int zeroes = 0;
    while (input.size() > 0 && input[0] == 0) {
        input = input.subspan(1);
        zeroes++;
    }

    int size = input.size() * 138 / 100 + 1;
    std::vector<unsigned char> b58(size);

    while (input.size() > 0) {
        int carry = input[0];
        int i = 0;

        for (auto it = b58.rbegin(); (carry != 0 || i < size) && (it != b58.rend()); it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        size = i;
        input = input.subspan(1);
    }

    auto it = b58.begin() + (size - zeroes);
    while (it != b58.end() && *it == 0)
        it++;

    std::string result;
    result.reserve(zeroes + (b58.end() - it));
    result.assign(zeroes, '1');
    while (it != b58.end())
        result += Base58Characters[*(it++)];
    return result;
}

// Encode a vector of bytes with a checksum into a Base58-encoded string
std::string EncodeBase58Check(Span<const unsigned char> input) {
    std::vector<unsigned char> vch(input.begin(), input.end());
    uint256 hash = Hash(vch);
    vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
    return EncodeBase58(vch);
}

// Decode a Base58-encoded string with a checksum into a vector of bytes
[[nodiscard]] static bool DecodeBase58Check(const char* str, std::vector<unsigned char>& result, int maxRetLength) {
    if (!DecodeBase58(str, result, std::min(maxRetLength > std::numeric_limits<int>::max() - 4 ? std::numeric_limits<int>::max() : maxRetLength + 4, std::numeric_limits<int>::max()))) {
        result.clear();
        return false;
    }

    if (result.size() < 4) {
        result.clear();
        return false;
    }

    uint256 hash = Hash(Span{result}.first(result.size() - 4));
    if (std::memcmp(&hash, &result[result.size() - 4], 4) != 0) {
        result.clear();
        return false;
    }

    result.resize(result.size() - 4);
    return true;
}

// Decode a Base58-encoded string with a checksum into a vector of bytes
bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& result, int maxRetLength) {
    if (!ContainsNoNUL(str))
        return false;
    return DecodeBase58Check(str.c_str(), result, maxRetLength);
}

