#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

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
};

class Event {
public:
    std::string Name;
    std::string RawName;
    bool Anonymous;
    Arguments Inputs;
    std::string str;
    std::string Sig;
    Hash ID;

    Event(const std::string& name, const std::string& rawName, bool anonymous, const Arguments& inputs) {
        Name = name;
        RawName = rawName;
        Anonymous = anonymous;
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

        str = "event " + RawName + "(" + Join(names, ", ") + ")";
        Sig = RawName + "(" + Join(types, ",") + ")";
        ID = Keccak256(Sig);
    }

    std::string String() const {
        return str;
    }

private:
    std::string Join(const std::vector<std::string>& elements, const std::string& delimiter) const {
        std::stringstream ss;
        std::copy(elements.begin(), elements.end(), std::ostream_iterator<std::string>(ss, delimiter.c_str()));
        std::string result = ss.str();
        return result.substr(0, result.length() - delimiter.length());
    }
};