// Copyright (c) [2024] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#include <crypto/common.h>
#include <span.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <stdint.h>
#include <string>

// Template class for fixed-sized opaque blobs
template<unsigned int BITS>
class base_blob
{
protected:
    // Width of the blob in bytes
    static constexpr int WIDTH = BITS / 8;
    // Array to hold blob data
    std::array<uint8_t, WIDTH> m_data;
    // Sanity check for array size
    static_assert(WIDTH == sizeof(m_data), "Sanity check");

public:
    // Default constructor, initializes with zero
    constexpr base_blob() : m_data() {}

    // Constructor for a constant value between 1 and 255
    constexpr explicit base_blob(uint8_t v) : m_data{v} {}

    // Constructor from a span of bytes
    constexpr explicit base_blob(Span<const unsigned char> vch)
    {
        assert(vch.size() == WIDTH);
        std::copy(vch.begin(), vch.end(), m_data.begin());
    }

    // Checks if the blob is null (all bytes are zero)
    constexpr bool IsNull() const
    {
        return std::all_of(m_data.begin(), m_data.end(), [](uint8_t val) {
            return val == 0;
        });
    }

    // Sets all bytes of the blob to zero
    constexpr void SetNull()
    {
        std::fill(m_data.begin(), m_data.end(), 0);
    }

    // Compares two blobs
    constexpr int Compare(const base_blob& other) const { return std::memcmp(m_data.data(), other.m_data.data(), WIDTH); }

    // Overloading comparison operators
    friend constexpr bool operator==(const base_blob& a, const base_blob& b) { return a.Compare(b) == 0; }
    friend constexpr bool operator!=(const base_blob& a, const base_blob& b) { return a.Compare(b) != 0; }
    friend constexpr bool operator<(const base_blob& a, const base_blob& b) { return a.Compare(b) < 0; }

    // Converts the blob to hexadecimal string
    std::string GetHex() const;
    // Sets the blob from a hexadecimal string
    void SetHex(const char* psz);
    // Sets the blob from a hexadecimal string
    void SetHex(const std::string& str);
    // Converts the blob to string
    std::string ToString() const;

    // Returns a pointer to the underlying data
    constexpr const unsigned char* data() const { return m_data.data(); }
    // Returns a pointer to the underlying data
    constexpr unsigned char* data() { return m_data.data(); }

    // Returns an iterator to the beginning of the data
    constexpr unsigned char* begin() { return m_data.data(); }
    // Returns an iterator to the end of the data
    constexpr unsigned char* end() { return m_data.data() + WIDTH; }

    // Returns a const iterator to the beginning of the data
    constexpr const unsigned char* begin() const { return m_data.data(); }
    // Returns a const iterator to the end of the data
    constexpr const unsigned char* end() const { return m_data.data() + WIDTH; }

    // Returns the width of the blob in bytes
    static constexpr unsigned int size() { return WIDTH; }

    // Gets a 64-bit integer from a specified position in the blob
    constexpr uint64_t GetUint64(int pos) const { return ReadLE64(m_data.data() + pos * 8); }

    // Serializes the blob to a stream
    template<typename Stream>
    void Serialize(Stream& s) const
    {
        s.write(MakeByteSpan(m_data));
    }

    // Unserializes the blob from a stream
    template<typename Stream>
    void Unserialize(Stream& s)
    {
        s.read(MakeWritableByteSpan(m_data));
    }
};

// 160-bit opaque blob
class uint160 : public base_blob<160> {
public:
    constexpr uint160() = default;
    constexpr explicit uint160(Span<const unsigned char> vch) : base_blob<160>(vch) {}
};

// 256-bit opaque blob
class uint256 : public base_blob<256> {
public:
    constexpr uint256() = default;
    constexpr explicit uint256(uint8_t v) : base_blob<256>(v) {}
    constexpr explicit uint256(Span<const unsigned char> vch) : base_blob<256>(vch) {}
    // Constants for zero and one
    static const uint256 ZERO;
    static const uint256 ONE;
};

// Constructs a uint256 from a hexadecimal string
inline uint256 uint256S(const char *str)
{
    uint256 rv;
    rv.SetHex(str);
    return rv;
}

// Constructs a uint256 from a hexadecimal string
inline uint256 uint256S(const std::string& str)
{
    uint256 rv;
    rv.SetHex(str);
    return rv;
}
