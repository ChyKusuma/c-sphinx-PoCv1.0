// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#ifndef SPHINX_UINT256_HPP
#define SPHINX_UINT256_HPP

#include <cstring>
#include <limits>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <array>

// Forward declarations
class uint256;
class uint_error;

/**
 * Custom exception class for uint256 operations.
 */
class uint_error : public std::runtime_error {
public:
    explicit uint_error(const std::string& str) : std::runtime_error(str) {}
};

/**
 * Template base class for unsigned big integers.
 */
template<unsigned int BITS>
class base_uint {
protected:
    static_assert(BITS / 32 > 0 && BITS % 32 == 0, "Template parameter BITS must be a positive multiple of 32.");
    static constexpr int WIDTH = BITS / 32;
    std::array<uint32_t, WIDTH> pn;

public:
    // Constructors
    base_uint();
    base_uint(const base_uint& b);
    base_uint& operator=(const base_uint& b);
    base_uint(uint64_t b);
    explicit base_uint(const std::string& str);

    // Unary operators
    base_uint operator~() const;
    base_uint operator-() const;

    // Utility functions
    double getdouble() const;
    std::string GetHex() const;
    void SetHex(const char* psz);
    void SetHex(const std::string& str);
    std::string ToString() const;
    unsigned int size() const;
    unsigned int bits() const;
    uint64_t GetLow64() const;

    // Assignment operators
    base_uint& operator=(uint64_t b);
    base_uint& operator^=(const base_uint& b);
    base_uint& operator&=(const base_uint& b);
    base_uint& operator|=(const base_uint& b);
    base_uint& operator^=(uint64_t b);
    base_uint& operator|=(uint64_t b);
    base_uint& operator<<=(unsigned int shift);
    base_uint& operator>>=(unsigned int shift);
    base_uint& operator+=(const base_uint& b);
    base_uint& operator-=(const base_uint& b);
    base_uint& operator+=(uint64_t b64);
    base_uint& operator-=(uint64_t b64);
    base_uint& operator*=(uint32_t b32);
    base_uint& operator*=(const base_uint& b);
    base_uint& operator/=(const base_uint& b);
    base_uint& operator++();
    base_uint operator++(int);
    base_uint& operator--();
    base_uint operator--(int);

    // Comparison operators
    int CompareTo(const base_uint& b) const;
    bool EqualTo(uint64_t b) const;
    friend bool operator==(const base_uint& a, const base_uint& b);
    friend bool operator!=(const base_uint& a, const base_uint& b);
    friend bool operator>(const base_uint& a, const base_uint& b);
    friend bool operator<(const base_uint& a, const base_uint& b);
    friend bool operator>=(const base_uint& a, const base_uint& b);
    friend bool operator<=(const base_uint& a, const base_uint& b);
    friend bool operator==(const base_uint& a, uint64_t b);
    friend bool operator!=(const base_uint& a, uint64_t b);

    // Friend functions
    friend base_uint operator+(const base_uint& a, const base_uint& b);
    friend base_uint operator-(const base_uint& a, const base_uint& b);
    friend base_uint operator*(const base_uint& a, const base_uint& b);
    friend base_uint operator/(const base_uint& a, const base_uint& b);
    friend base_uint operator|(const base_uint& a, const base_uint& b);
    friend base_uint operator&(const base_uint& a, const base_uint& b);
    friend base_uint operator^(const base_uint& a, const base_uint& b);
    friend base_uint operator>>(const base_uint& a, int shift);
    friend base_uint operator<<(const base_uint& a, int shift);
    friend base_uint operator*(const base_uint& a, uint32_t b);

};

/**
 * 256-bit unsigned big integer.
 */
class arith_uint256 : public base_uint<256> {
public:
    // Constructors
    arith_uint256() {}
    arith_uint256(const base_uint<256>& b) : base_uint<256>(b) {}
    arith_uint256(uint64_t b) : base_uint<256>(b) {}
    explicit arith_uint256(const std::string& str) : base_uint<256>(str) {}

    // Compact representation functions
    arith_uint256& SetCompact(uint32_t nCompact, bool *pfNegative = nullptr, bool *pfOverflow = nullptr);
    uint32_t GetCompact(bool fNegative = false) const;

    // Friend functions
    friend uint256 ArithToUint256(const arith_uint256 &);
    friend arith_uint256 UintToArith256(const uint256 &);
};

// Forward declarations
uint256 ArithToUint256(const arith_uint256 &);
arith_uint256 UintToArith256(const uint256 &);

// External template declaration
extern template class base_uint<256>;

/**
 * Custom 256-bit unsigned integer class.
 */
template <size_t N>
class uint256 {
private:
    std::array<uint64_t, N> data;

public:
    // Constructors
    uint256();
    uint256(uint64_t value);

    // Arithmetic operators
    uint256 operator+(const uint256& other) const;
    uint256 operator-(const uint256& other) const;
    uint256 operator&(const uint256& other) const;
    uint256 operator|(const uint256& other) const;
    uint256 operator^(const uint256& other) const;

    // Comparison operators
    bool operator==(const uint256& other) const;
    bool operator!=(const uint256& other) const;
    bool operator<(const uint256& other) const;
    bool operator>(const uint256& other) const;
    bool operator<=(const uint256& other) const;
    bool operator>=(const uint256& other) const;

    // Compact representation functions
    uint256& SetCompact(uint32_t nCompact, bool* pfNegative, bool* pfOverflow);
    uint32_t GetCompact(bool fNegative) const;
};

#include "uint256.tpp"

#endif // SPHINX_UINT256_HPP

