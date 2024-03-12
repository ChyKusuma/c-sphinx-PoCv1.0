// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#include <uint256.hpp>
#include <util/strencodings.h>

// Template specialization for GetHex method of base_blob class
template <unsigned int BITS>
std::string base_blob<BITS>::GetHex() const
{
    // Reverse the data
    std::array<uint8_t, WIDTH> reversedData;
    for (int i = 0; i < WIDTH; ++i) {
        reversedData[i] = m_data[WIDTH - 1 - i];
    }
    
    // Convert to hex string
    return HexStr(reversedData);
}

// Template specialization for SetHex method of base_blob class
template <unsigned int BITS>
void base_blob<BITS>::SetHex(const char* psz)
{
    // Clear data
    std::fill(m_data.begin(), m_data.end(), 0);

    // Skip leading spaces
    while (IsSpace(*psz))
        psz++;

    // Skip "0x" prefix
    if (psz[0] == '0' && ToLower(psz[1]) == 'x')
        psz += 2;

    // Convert hex string to binary data
    size_t digits = 0;
    while (::HexDigit(psz[digits]) != -1)
        digits++;
    unsigned char* p1 = m_data.data();
    unsigned char* pend = p1 + WIDTH;
    while (digits > 0 && p1 < pend) {
        *p1 = ::HexDigit(psz[--digits]);
        if (digits > 0) {
            *p1 |= ((unsigned char)::HexDigit(psz[--digits]) << 4);
            p1++;
        }
    }
}

// Template specialization for SetHex method of base_blob class
template <unsigned int BITS>
void base_blob<BITS>::SetHex(const std::string& str)
{
    SetHex(str.c_str());
}

// Template specialization for ToString method of base_blob class
template <unsigned int BITS>
std::string base_blob<BITS>::ToString() const
{
    return GetHex();
}

// Explicit instantiations for base_blob<160>
template std::string base_blob<160>::GetHex() const;
template std::string base_blob<160>::ToString() const;
template void base_blob<160>::SetHex(const char*);
template void base_blob<160>::SetHex(const std::string&);

// Explicit instantiations for base_blob<256>
template std::string base_blob<256>::GetHex() const;
template std::string base_blob<256>::ToString() const;
template void base_blob<256>::SetHex(const char*);
template void base_blob<256>::SetHex(const std::string&);

// Definitions for static constants in uint256 class
const uint256 uint256::ZERO(0);
const uint256 uint256::ONE(1);
