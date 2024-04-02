#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <array>

// Dummy implementation of common.LeftPadBytes and common.RightPadBytes for demonstration purposes
std::vector<uint8_t> LeftPadBytes(const std::vector<uint8_t>& bytes, size_t size) {
    if (bytes.size() >= size) {
        return bytes;
    }
    std::vector<uint8_t> padded(size - bytes.size(), 0);
    padded.insert(padded.end(), bytes.begin(), bytes.end());
    return padded;
}

std::vector<uint8_t> RightPadBytes(const std::vector<uint8_t>& bytes, size_t size) {
    if (bytes.size() >= size) {
        return bytes;
    }
    std::vector<uint8_t> padded = bytes;
    padded.insert(padded.end(), size - bytes.size(), 0);
    return padded;
}

// Dummy implementation of math.PaddedBigBytes for demonstration purposes
std::vector<uint8_t> PaddedBigBytes(uint64_t num, size_t size) {
    std::ostringstream oss;
    oss << std::setw(size * 2) << std::setfill('0') << std::hex << num;
    std::string hex = oss.str();
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        bytes.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
    }
    return bytes;
}

// Dummy implementation of math.U256Bytes for demonstration purposes
std::vector<uint8_t> U256Bytes(const std::vector<uint8_t>& num) {
    return num;
}

std::vector<uint8_t> U256Bytes(const std::string& num) {
    std::vector<uint8_t> bytes;
    for (char c : num) {
        bytes.push_back(static_cast<uint8_t>(c));
    }
    return RightPadBytes(bytes, 32);
}

// Dummy implementation of mustArrayToByteSlice for demonstration purposes
std::vector<uint8_t> mustArrayToByteSlice(const std::vector<uint8_t>& arr) {
    return arr;
}

// Type enum definition
enum Type {
    IntTy,
    UintTy,
    StringTy,
    AddressTy,
    BoolTy,
    BytesTy,
    FixedBytesTy,
    FunctionTy
};

// Dummy implementation of Type struct for demonstration purposes
struct Type {
    Type(Type t) : T(t) {}
    Type(Type t, size_t s) : T(t), Size(s) {}
    Type T;
    size_t Size;
};

// Dummy implementation of reflect.Value for demonstration purposes
template<typename T>
class Value {
public:
    Value(T val) : value(val) {}
    T get() const { return value; }
private:
    T value;
};

// Dummy implementation of reflect.Value.Kind for demonstration purposes
enum Kind {
    Uint,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Int,
    Int8,
    Int16,
    Int32,
    Int64,
    Ptr
};

// Dummy implementation of reflect.Value.Kind for demonstration purposes
Kind reflectValueKind(Value<uint64_t> val) {
    return Uint64;
}

Kind reflectValueKind(Value<int64_t> val) {
    return Int64;
}

template<typename T>
Kind reflectValueKind(Value<T*> val) {
    return Ptr;
}

// packBytesSlice packs the given bytes as [L, V] as the canonical representation
// bytes slice.
std::vector<uint8_t> packBytesSlice(const std::vector<uint8_t>& bytes, size_t l) {
    std::vector<uint8_t> len = U256Bytes(l);
    return len + RightPadBytes(bytes, (l + 31) / 32 * 32);
}

// packElement packs the given reflect value according to the abi specification in
// t.
std::vector<uint8_t> packElement(Type t, const Value<uint64_t>& reflectValue) {
    switch (t.T) {
    case IntTy:
    case UintTy:
        return U256Bytes(reflectValue.get());
    case StringTy:
        return packBytesSlice(U256Bytes(std::to_string(reflectValue.get())), reflectValue.get());
    case AddressTy:
        return LeftPadBytes(U256Bytes(reflectValue.get()), 32);
    case BoolTy:
        return reflectValue.get() ? PaddedBigBytes(1, 32) : PaddedBigBytes(0, 32);
    case BytesTy:
        return packBytesSlice(reflectValue.get(), reflectValue.get().size());
    case FixedBytesTy:
    case FunctionTy:
        return RightPadBytes(reflectValue.get(), 32);
    default:
        throw std::runtime_error("Could not pack element, unknown type");
    }
}

std::vector<uint8_t> packElement(Type t, const Value<int64_t>& reflectValue) {
    switch (t.T) {
    case IntTy:
    case UintTy:
        return U256Bytes(reflectValue.get());
    default:
        throw std::runtime_error("Could not pack element, unknown type");
    }
}

template<typename T>
std::vector<uint8_t> packNum(const Value<T>& value) {
    switch (reflectValueKind(value)) {
    case Uint64:
        return U256Bytes(value.get());
    case Int64:
        return U256Bytes(static_cast<uint64_t>(value.get()));
    case Ptr:
        return U256Bytes(static_cast<uint64_t>(*value.get()));
    default:
        throw std::runtime_error("abi: fatal error");
    }
}