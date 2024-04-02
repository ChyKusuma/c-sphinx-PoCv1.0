#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>
#include <type_traits>

enum Type {
    UintTy,
    IntTy,
    BoolTy,
    StringTy,
    BytesTy,
    ArrayTy,
    TupleTy,
    SliceTy,
    FixedBytesTy
};

struct TypeStruct {
    Type T;
    int Size;
    Type Elem;
};

class ABI {
public:
    std::string formatSliceString(Type kind, int sliceSize) {
        if (sliceSize == -1) {
            return "[]" + getTypeString(kind);
        }
        std::stringstream ss;
        ss << "[" << sliceSize << "]" << getTypeString(kind);
        return ss.str();
    }

    std::string getTypeString(Type type) {
        switch (type) {
            case UintTy: return "uint";
            case IntTy: return "int";
            case BoolTy: return "bool";
            case StringTy: return "string";
            case BytesTy: return "bytes";
            case ArrayTy: return "array";
            case TupleTy: return "tuple";
            case SliceTy: return "slice";
            case FixedBytesTy: return "fixed bytes";
            default: return "unknown";
        }
    }

    std::string getTypeString(TypeStruct t) {
        if (t.T == ArrayTy) {
            return "[" + std::to_string(t.Size) + "]" + getTypeString(t.Elem);
        } else if (t.T == SliceTy) {
            return "[]" + getTypeString(t.Elem);
        } else if (t.T == FixedBytesTy) {
            return "fixed bytes[" + std::to_string(t.Size) + "]";
        } else {
            return getTypeString(t.T);
        }
    }

    std::string getTypeString(std::vector<TypeStruct> types) {
        std::string result = "{";
        for (size_t i = 0; i < types.size(); ++i) {
            result += getTypeString(types[i]);
            if (i < types.size() - 1) {
                result += ", ";
            }
        }
        result += "}";
        return result;
    }

    std::string getTypeString(std::array<TypeStruct, 2> types) {
        return "(" + getTypeString(types[0]) + ", " + getTypeString(types[1]) + ")";
    }

    std::string getTypeString(std::array<TypeStruct, 3> types) {
        return "(" + getTypeString(types[0]) + ", " + getTypeString(types[1]) + ", " + getTypeString(types[2]) + ")";
    }

    std::string getTypeString(TypeStruct* types, size_t size) {
        std::string result = "{";
        for (size_t i = 0; i < size; ++i) {
            result += getTypeString(types[i]);
            if (i < size - 1) {
                result += ", ";
            }
        }
        result += "}";
        return result;
    }

    std::string getTypeString(std::vector<Type> types) {
        std::string result = "{";
        for (size_t i = 0; i < types.size(); ++i) {
            result += getTypeString(types[i]);
            if (i < types.size() - 1) {
                result += ", ";
            }
        }
        result += "}";
        return result;
    }

    std::string getTypeString(std::array<Type, 2> types) {
        return "(" + getTypeString(types[0]) + ", " + getTypeString(types[1]) + ")";
    }

    std::string getTypeString(std::array<Type, 3> types) {
        return "(" + getTypeString(types[0]) + ", " + getTypeString(types[1]) + ", " + getTypeString(types[2]) + ")";
    }

    std::string getTypeString(Type* types, size_t size) {
        std::string result = "{";
        for (size_t i = 0; i < size; ++i) {
            result += getTypeString(types[i]);
            if (i < size - 1) {
                result += ", ";
            }
        }
        result += "}";
        return result;
    }

    std::string typeErr(const std::string& expected, const std::string& got) {
        return "abi: cannot use " + got + " as type " + expected + " as argument";
    }

    std::string typeErr(Type expected, Type got) {
        return typeErr(getTypeString(expected), getTypeString(got));
    }

    std::string typeErr(const std::string& expected, const std::string& got, const std::string& val) {
        return "abi: cannot use " + got + " as type " + expected + " as argument value " + val;
    }

    std::string typeErr(Type expected, Type got, const std::string& val) {
        return typeErr(getTypeString(expected), getTypeString(got), val);
    }

    template<typename T>
    std::string typeErr(Type expected, T got) {
        return typeErr(getTypeString(expected), typeid(T).name(), std::to_string(got));
    }

    template<typename T, typename U>
    std::string typeErr(T expected, U got) {
        return typeErr(expected, got, "");
    }

    template<typename T, typename U>
    std::string typeErr(T expected, U got, const std::string& val) {
        return typeErr(getTypeString(expected), getTypeString(got), val);
    }

    template<typename T>
    std::string typeErr(const std::string& expected, const T& got) {
        return typeErr(expected, typeid(T).name(), std::to_string(got));
    }

    template<typename T, typename U>
    std::string typeErr(const T& expected, const U& got) {
        return typeErr(expected, got, "");
    }

    template<typename T, typename U>
    std::string typeErr(const T& expected, const U& got, const std::string& val) {
        return typeErr(getTypeString(expected), getTypeString(got), val);
    }
};