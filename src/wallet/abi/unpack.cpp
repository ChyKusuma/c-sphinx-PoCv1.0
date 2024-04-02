#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include <memory>
#include <map>

class Type {
public:
    int Size;
    uint8_t T;

    bool requiresLengthPrefix() {
        return T == SliceTy || T == ArrayTy || T == StringTy;
    }

    std::type_info getType() {
        switch (T) {
            case IntTy:
            case UintTy:
                return typeid(int);
            case BoolTy:
                return typeid(bool);
            case StringTy:
                return typeid(std::string);
            case SliceTy:
            case ArrayTy:
                return typeid(std::vector<unsigned char>);
            default:
                throw std::runtime_error("Unknown type");
        }
    }
};

constexpr uint8_t UintTy = 1;
constexpr uint8_t IntTy = 2;
constexpr uint8_t BoolTy = 3;
constexpr uint8_t StringTy = 4;
constexpr uint8_t SliceTy = 5;
constexpr uint8_t ArrayTy = 6;
constexpr uint8_t TupleTy = 7;
constexpr uint8_t AddressTy = 8;
constexpr uint8_t FixedBytesTy = 9;
constexpr uint8_t BytesTy = 10;
constexpr uint8_t HashTy = 11;
constexpr uint8_t FunctionTy = 12;

class common {
public:
    class Address {};
    static const std::string Big1;
    static const std::string Big32;
};

const std::string common::Big1 = "1";
const std::string common::Big32 = "32";

std::vector<unsigned char> ReadInteger(Type typ, std::vector<unsigned char> b) {
    std::vector<unsigned char> result;
    if (typ.T == UintTy) {
        switch (typ.Size) {
            case 8:
                result.push_back(b.back());
                break;
            case 16:
                // Assuming big-endian encoding
                result.push_back(b[b.size() - 2]);
                result.push_back(b[b.size() - 1]);
                break;
            case 32:
                // Assuming big-endian encoding
                result.push_back(b[b.size() - 4]);
                result.push_back(b[b.size() - 3]);
                result.push_back(b[b.size() - 2]);
                result.push_back(b[b.size() - 1]);
                break;
            case 64:
                // Assuming big-endian encoding
                for (int i = b.size() - 8; i < b.size(); ++i) {
                    result.push_back(b[i]);
                }
                break;
            default:
                // the only case left for unsigned integer is uint256.
                result = b;
        }
    }
    return result;
}

bool readBool(std::vector<unsigned char> word) {
    for (size_t i = 0; i < 31; ++i) {
        if (word[i] != 0) {
            return false;
        }
    }
    return word[31] != 0;
}

std::array<unsigned char, 24> readFunctionType(Type t, std::vector<unsigned char> word) {
    std::array<unsigned char, 24> funcTy;
    if (t.T != FunctionTy) {
        throw std::runtime_error("Invalid type in call to make function type byte array");
    }
    if (word.size() < 32) {
        throw std::runtime_error("Got improperly encoded function type");
    }
    for (size_t i = 0; i < 24; ++i) {
        funcTy[i] = word[i];
    }
    return funcTy;
}

std::string ReadFixedBytes(Type t, std::vector<unsigned char> word) {
    if (t.T != FixedBytesTy) {
        throw std::runtime_error("Invalid type in call to make fixed byte array");
    }
    std::string array(word.begin(), word.begin() + t.Size);
    return array;
}

std::vector<unsigned char> forEachUnpack(Type t, std::vector<unsigned char> output, int start, int size) {
    if (size < 0) {
        throw std::runtime_error("Cannot marshal input to array, size is negative");
    }
    if (start + 32 * size > output.size()) {
        throw std::runtime_error("Cannot marshal in to go array: offset would go over slice boundary");
    }

    std::vector<unsigned char> refSlice;

    for (int i = start, j = 0; j < size; i += 32, ++j) {
        auto inter = toGoType(i, t, output);
        refSlice.insert(refSlice.end(), inter.begin(), inter.end());
    }

    return refSlice;
}

std::vector<unsigned char> forTupleUnpack(Type t, std::vector<unsigned char> output) {
    std::vector<unsigned char> retval;
    size_t virtualArgs = 0;
    for (size_t index = 0; index < t.Size; ++index) {
        auto elem = t.TupleElems[index];
        auto marshalledValue = toGoType((index + virtualArgs) * 32, *elem, output);
        if (elem.T == ArrayTy && !isDynamicType(*elem)) {
            virtualArgs += getTypeSize(*elem) / 32 - 1;
        } else if (elem.T == TupleTy && !isDynamicType(*elem)) {
            virtualArgs += getTypeSize(*elem) / 32 - 1;
        }
        retval.insert(retval.end(), marshalledValue.begin(), marshalledValue.end());
    }
    return retval;
}

std::vector<unsigned char> toGoType(int index, Type t, std::vector<unsigned char> output) {
    std::vector<unsigned char> returnOutput;
    int begin = 0, length = 0;

    if (t.requiresLengthPrefix()) {
        std::tie(begin, length) = lengthPrefixPointsTo(index, output);
    } else {
        returnOutput.assign(output.begin() + index, output.begin() + index + 32);
    }

    switch (t.T) {
        case TupleTy:
            if (isDynamicType(t)) {
                begin = tuplePointsTo(index, output);
                return forTupleUnpack(t, std::vector<unsigned char>(output.begin() + begin, output.end()));
            }
            return forTupleUnpack(t, std::vector<unsigned char>(output.begin() + index, output.end()));
        case SliceTy:
            return forEachUnpack(t, std::vector<unsigned char>(output.begin() + begin, output.end()), 0, length);
        case ArrayTy:
            if (isDynamicType(*t.Elem)) {
                int offset = binaryBigEndianToUint64(returnOutput);
                if (offset > output.size()) {
                    throw std::runtime_error("ToGoType offset greater than output length");
                }
                return forEachUnpack(t, std::vector<unsigned char>(output.begin() + offset, output.end()), 0, t.Size);
            }
            return forEachUnpack(t, std::vector<unsigned char>(output.begin() + index, output.end()), 0, t.Size);
        case StringTy:
            return std::vector<unsigned char>(output.begin() + begin, output.begin() + begin + length);
        case IntTy:
        case UintTy:
            return ReadInteger(t, returnOutput);
        case BoolTy:
            return readBool(returnOutput) ? std::vector<unsigned char>{1} : std::vector<unsigned char>{0};
        case AddressTy:
            return std::vector<unsigned char>(returnOutput.begin() + begin, returnOutput.begin() + begin + 20);
        case FixedBytesTy:
            return std::vector<unsigned char>(returnOutput.begin(), returnOutput.begin() + t.Size);
        case FunctionTy:
            return readFunctionType(t, returnOutput);
        default:
            throw std::runtime_error("Unknown type");
    }
}

std::pair<int, int> lengthPrefixPointsTo(int index, std::vector<unsigned char> output) {
    auto offset = binaryBigEndianToUint64(std::vector<unsigned char>(output.begin() + index, output.begin() + index + 32));
    auto offsetEnd = offset + binaryBigEndianToUint64(std::vector<unsigned char>(output.begin() + offset - 32, output.begin() + offset));
    if (offsetEnd > output.size()) {
        throw std::runtime_error("Cannot marshal in to go slice: offset would go over slice boundary");
    }
    return {offset, static_cast<int>(binaryBigEndianToUint64(std::vector<unsigned char>(output.begin() + offset - 32, output.begin() + offset)))};
}

int binaryBigEndianToUint64(std::vector<unsigned char> vec) {
    uint64_t value = 0;
    for (size_t i = 0; i < vec.size(); ++i) {
        value |= static_cast<uint64_t>(vec[i]) << (8 * (vec.size() - i - 1));
    }
    return value;
}

int tuplePointsTo(int index, std::vector<unsigned char> output) {
    auto offset = binaryBigEndianToUint64(std::vector<unsigned char>(output.begin() + index, output.begin() + index + 32));
    if (offset > output.size()) {
        throw std::runtime_error("Cannot marshal in to go slice: offset would go over slice boundary");
    }
    return offset;
}

bool isDynamicType(Type t) {
    if (t.T == TupleTy) {
        for (auto& elem : t.TupleElems) {
            if (isDynamicType(*elem)) {
                return true;
            }
        }
        return false;
    }
    return t.T == StringTy || t.T == BytesTy || t.T == SliceTy || (t.T == ArrayTy && isDynamicType(*t.Elem));
}

int getTypeSize(Type t) {
    if (t.T == ArrayTy && !isDynamicType(*t.Elem)) {
        if (t.Elem->T == ArrayTy || t.Elem->T == TupleTy) {
            return t.Size * getTypeSize(*t.Elem);
        }
        return t.Size * 32;
    } else if (t.T == TupleTy && !isDynamicType(t)) {
        int total = 0;
        for (auto& elem : t.TupleElems) {
            total += getTypeSize(*elem);
        }
        return total;
    }
    return 32;
}

bool isValidFieldName(const std::string& fieldName) {
    for (size_t i = 0; i < fieldName.size(); ++i) {
        if (i == 0 && !isLetter(fieldName[i])) {
            return false;
        }
        if (!(isLetter(fieldName[i]) || std::isdigit(fieldName[i]))) {
            return false;
        }
    }
    return !fieldName.empty();
}

int main() {
    try {
        // Example usage:
        Type type;
        type.Size = 256;
        type.T = UintTy;
        std::vector<unsigned char> b = {0xFF};
        auto result = ReadInteger(type, b);
        for (auto& byte : result) {
            std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}