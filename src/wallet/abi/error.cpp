#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of common.Hash and crypto.Keccak256 for demonstration purposes
class Hash {
public:
    Hash(const std::string& hex) : hash_(hex) {}
    std::string Hex() const { return hash_; }
private:
    std::string hash_;
};

Hash Keccak256(const std::string& data) {
    // Dummy implementation, just returns a hash of the input string
    return Hash(data + "_hash");
}

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

struct Argument {
    std::string Name;
    Type Type;
    bool Indexed;
};

class Arguments {
public:
    std::vector<Argument> args;
    
    std::vector<std::string> Unpack(const std::vector<char>& data) {
        // Dummy implementation, just returns the hexadecimal representation of the input data
        std::vector<std::string> result;
        for (const char& c : data) {
            result.push_back(std::string(1, c));
        }
        return result;
    }
};

class Error {
public:
    std::string Name;
    Arguments Inputs;
    std::string str;
    std::string Sig;
    Hash ID;

    Error(const std::string& name, const Arguments& inputs) {
        Name = name;
        Inputs = inputs;

        std::vector<std::string> names;
        std::vector<std::string> types;

        for (size_t i = 0; i < Inputs.args.size(); ++i) {
            const Argument& input = Inputs.args[i];
            if (input.Name.empty()) {
                // Sanitize inputs without names
                Argument sanitizedInput = {"arg" + std::to_string(i), input.Type, input.Indexed};
                Inputs.args[i] = sanitizedInput;
            }
            // String representation
            std::string nameStr = input.Type + " " + input.Name;
            if (input.Indexed) {
                nameStr = input.Type + " indexed " + input.Name;
            }
            names.push_back(nameStr);
            // Sig representation
            types.push_back(std::to_string(input.Type));
        }

        str = "error " + Name + "(" + Join(names, ", ") + ")";
        Sig = Name + "(" + Join(types, ",") + ")";
        ID = Keccak256(Sig);
    }

    std::string String() const {
        return str;
    }

    std::pair<std::string, std::vector<std::string>> Unpack(const std::vector<char>& data) const {
        if (data.size() < 4) {
            throw std::runtime_error("invalid data for unpacking");
        }
        std::vector<char> firstFourBytes(data.begin(), data.begin() + 4);
        std::string firstFourBytesStr(firstFourBytes.begin(), firstFourBytes.end());
        if (firstFourBytesStr != ID.Hex().substr(0, 4)) {
            throw std::runtime_error("invalid data for unpacking");
        }
        return std::make_pair(std::to_string(ID.Hex()), Inputs.Unpack(std::vector<char>(data.begin() + 4, data.end())));
    }

private:
    std::string Join(const std::vector<std::string>& elements, const std::string& delimiter) const {
        std::stringstream ss;
        std::copy(elements.begin(), elements.end(), std::ostream_iterator<std::string>(ss, delimiter.c_str()));
        std::string result = ss.str();
        return result.substr(0, result.length() - delimiter.length());
    }
};