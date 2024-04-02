#include <vector>
#include <string>
#include <stdexcept>
#include <regex>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <memory>
#include <map>
#include <cctype>

constexpr uint8_t IntTy = 0;
constexpr uint8_t UintTy = 1;
constexpr uint8_t BoolTy = 2;
constexpr uint8_t StringTy = 3;
constexpr uint8_t SliceTy = 4;
constexpr uint8_t ArrayTy = 5;
constexpr uint8_t TupleTy = 6;
constexpr uint8_t AddressTy = 7;
constexpr uint8_t FixedBytesTy = 8;
constexpr uint8_t BytesTy = 9;
constexpr uint8_t HashTy = 10;
constexpr uint8_t FixedPointTy = 11;
constexpr uint8_t FunctionTy = 12;

class Type {
public:
    Type* Elem;
    int Size;
    uint8_t T;
    std::string stringKind;
    std::string TupleRawName;
    std::vector<Type*> TupleElems;
    std::vector<std::string> TupleRawNames;
    std::type_info TupleType;
};

class common {
public:
    class Address {};
};

class ArgumentMarshaling {
public:
    std::string Type;
    std::string InternalType;
    std::vector<ArgumentMarshaling> Components;
};

static const std::regex typeRegex("([a-zA-Z]+)(([0-9]+)(x([0-9]+))?)?");

Type NewType(const std::string& t, const std::string& internalType, const std::vector<ArgumentMarshaling>& components) {
    Type typ;
    typ.stringKind = t;

    if (std::count(t.begin(), t.end(), '[') != std::count(t.begin(), t.end(), ']')) {
        throw std::runtime_error("Invalid arg type in abi");
    }

    if (std::count(t.begin(), t.end(), '[') != 0) {
        std::string subInternal = internalType;
        auto i = subInternal.find_last_of('[');
        if (i != std::string::npos) {
            subInternal = subInternal.substr(0, i);
        }
        auto embeddedType = NewType(t.substr(0, t.find_last_of('[')), subInternal, components);
        auto sliced = t.substr(t.find_last_of('['));

        std::regex re("[0-9]+");
        std::sregex_iterator it(sliced.begin(), sliced.end(), re);
        std::sregex_iterator end;
        auto intz = std::vector<std::string>();
        for (; it != end; ++it) {
            intz.push_back(it->str());
        }

        if (intz.empty()) {
            typ.T = SliceTy;
            typ.Elem = &embeddedType;
            typ.stringKind = embeddedType.stringKind + sliced;
        } else if (intz.size() == 1) {
            typ.T = ArrayTy;
            typ.Elem = &embeddedType;
            typ.Size = std::stoi(intz[0]);
            typ.stringKind = embeddedType.stringKind + sliced;
        } else {
            throw std::runtime_error("Invalid formatting of array type");
        }

        return typ;
    }

    std::smatch matches;
    if (!std::regex_search(t, matches, typeRegex)) {
        throw std::runtime_error("Invalid type '" + t + "'");
    }
    auto parsedType = matches;

    int varSize = 0;
    if (!parsedType[3].str().empty()) {
        varSize = std::stoi(parsedType[2].str());
    } else {
        if (parsedType[0] == "uint" || parsedType[0] == "int") {
            throw std::runtime_error("Unsupported arg type: " + t);
        }
    }

    auto varType = parsedType[1].str();
    if (varType == "int") {
        typ.Size = varSize;
        typ.T = IntTy;
    } else if (varType == "uint") {
        typ.Size = varSize;
        typ.T = UintTy;
    } else if (varType == "bool") {
        typ.T = BoolTy;
    } else if (varType == "address") {
        typ.Size = 20;
        typ.T = AddressTy;
    } else if (varType == "string") {
        typ.T = StringTy;
    } else if (varType == "bytes") {
        if (varSize == 0) {
            typ.T = BytesTy;
        } else {
            typ.T = FixedBytesTy;
            typ.Size = varSize;
        }
    } else if (varType == "tuple") {
        std::vector<std::pair<std::string, std::string>> fields;
        std::string expression = "(";
        for (const auto& c : components) {
            auto cType = NewType(c.Type, c.InternalType, c.Components);
            auto name = c.Name;
            if (name.empty()) {
                throw std::runtime_error("abi: purely anonymous or underscored field is not supported");
            }
            std::map<std::string, bool> used;
            auto fieldName = name;
            if (!isValidFieldName(fieldName)) {
                throw std::runtime_error("Field has invalid name");
            }
            fields.emplace_back(fieldName, cType.stringKind);
            expression += cType.stringKind;
            if (&c != &components.back()) {
                expression += ",";
            }
        }
        expression += ")";
        typ.TupleType = typeid(std::tuple<decltype(fields)::value_type>);
        for (const auto& [fieldName, fieldType] : fields) {
            auto field = NewType(fieldType, "", {});
            typ.TupleElems.push_back(&field);
            typ.TupleRawNames.push_back(fieldName);
        }
        typ.TupleRawName = expression;
        typ.T = TupleTy;
        typ.stringKind = expression;
    } else if (varType == "function") {
        typ.T = FunctionTy;
        typ.Size = 24;
    } else {
        throw std::runtime_error("Unsupported arg type: " + t);
    }

    return typ;
}

std::type_info GetType(const Type& t) {
    switch (t.T) {
        case IntTy:
        case UintTy:
            return typeid(int);
        case BoolTy:
            return typeid(bool);
        case StringTy:
            return typeid(std::string);
        case SliceTy:
        case ArrayTy:
            return typeid(std::vector<typename std::remove_pointer<decltype(t.Elem)>::type>);
        case TupleTy:
            return t.TupleType;
        case AddressTy:
            return typeid(common::Address);
        case FixedBytesTy:
            return typeid(std::array<unsigned char, t.Size>);
        case BytesTy:
            return typeid(std::vector<unsigned char>);
        default:
            throw std::runtime_error("Invalid type");
    }
}

std::string toHexString(const std::vector<unsigned char>& data) {
    std::string hexString;
    for (const auto& byte : data) {
        char hexBuffer[3];
        std::sprintf(hexBuffer, "%02X", byte);
        hexString += hexBuffer;
    }
    return hexString;
}

bool isDynamicType(const Type& t) {
    if (t.T == TupleTy) {
        for (const auto& elem : t.TupleElems) {
            if (isDynamicType(*elem)) {
                return true;
            }
        }
        return false;
    }
    return t.T == StringTy || t.T == BytesTy || t.T == SliceTy || (t.T == ArrayTy && isDynamicType(*t.Elem));
}

int getTypeSize(const Type& t) {
    if (t.T == ArrayTy && !isDynamicType(*t.Elem)) {
        if (t.Elem->T == ArrayTy || t.Elem->T == TupleTy) {
            return t.Size * getTypeSize(*t.Elem);
        }
        return t.Size * 32;
    } else if (t.T == TupleTy && !isDynamicType(t)) {
        int total = 0;
        for (const auto& elem : t.TupleElems) {
            total += getTypeSize(*elem);
        }
        return total;
    }
    return 32;
}

bool isLetter(char ch) {
    return std::isalpha(ch) || ch == '_' || std::isalpha(ch);
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
        auto type = NewType("uint256", "", {});
        std::cout << "Type: " << type.stringKind << ", Size: " << type.Size << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
