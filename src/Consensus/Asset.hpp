// Copyright (c) [2023] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#ifndef SPX_ASSET_HPP
#define SPX_ASSET_HPP

#include <cstdint>

namespace SPHINXAsset {

    /** Amount in Smix (Can be negative) */
    typedef int64_t CAmount;

    /**
        * 1 SPX (Symbolic Pixel) = 1,000,000,000,000,000,000 Smix (Smallest Symbolic Pixel)
        * 1 Gsmix (Giga-Smix)    = 1,000,000,000 Smix
        * 1 Msmix (Mega-Smix)    = 1,000,000 Smix
        * 1 ksmix (Kilo-Smix)    = 1,000 Smix
        * 1 Smix                 = The smallest unit
    **/

    /**
        * The biggest unit is 1 SPX (Symbolic Pixel). It's the highest denomination and represents 1 quintillion
        * (1,000,000,000,000,000,000) Smix (Smallest Symbolic Pixel).
        * The smallest unit is 1 Smix. It is the base unit and is the smallest denomination in system. All other
        * denominations are multiples of this base unit.
        * The denominations between the biggest and smallest units are as follows:
        * 1 GSPX (Giga-Smix) represents 1 billion Smix.
        * 1 MSPX (Mega-Smix) represents 1 million Smix.
        * 1 kSPX (Kilo-Smix) represents 1 thousand Smix.
    **/

    // Denominations of SPX
    static constexpr CAmount SPX  = 1000000000000000000;     // 1 SPX  = 1,000,000,000,000,000,000 Smix
    static constexpr CAmount GSPX = 1000000000000;           // 1 GSPX = 1,000,000,000 Smix
    static constexpr CAmount MSPX = 1000000000;              // 1 MSPX = 1,000,000 Smix
    static constexpr CAmount kSPX = 1000000;                 // 1 kSPX = 1,000 Smix
    static constexpr CAmount Smix = 1;                       // 1 Smix = 1 Smix

    /** Maximum supply of SPX */
    static constexpr uint64_t MAX_SUPPLY = 50000000ULL * SPX;

    /** Check if an amount of SPX is within valid range */
    inline bool SPXRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_SUPPLY); }

} // namespace SPHINXAsset

#endif // SPX_ASSET_HPP
