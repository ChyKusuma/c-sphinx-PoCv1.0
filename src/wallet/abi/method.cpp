#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

// Dummy implementation of crypto.Keccak256 for demonstration purposes
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

enum FunctionType {
    Constructor,
    Fallback,
    Receive,
    Function
};

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

class Method {
public:
    std::string Name;
    std::string RawName;
    FunctionType Type;
    std::string StateMutability;
    bool Constant;
    bool Payable;
    Arguments Inputs;
    Arguments Outputs;
    std::string str;
    std::string Sig;
    std::vector<uint8_t> ID;

    Method(const std::string& name, const std::string& rawName, FunctionType funType, const std::string& mutability, bool isConst, bool isPayable, const Arguments& inputs, const Arguments& outputs) {
        Name = name;
        RawName = rawName;
        Type = funType;
        StateMutability = mutability;
        Constant = isConst;
        Payable = isPayable;
        Inputs = inputs;
        Outputs = outputs;

        std::vector<std::string> types(inputs.args.size());
        std::vector<std::string> inputNames(inputs.args.size());
        std::vector<std::string> outputNames(outputs.args.size());

        for (size_t i = 0; i < inputs.args.size(); ++i) {
            const Argument& input = inputs.args[i];
            inputNames[i] = input.Type + " " + input.Name;
            types[i] = std::to_string(input.Type);
        }

        for (size_t i = 0; i < outputs.args.size(); ++i) {
            const Argument& output = outputs.args[i];
            outputNames[i] = output.Type;
            if (!output.Name.empty()) {
                outputNames[i] += " " + output.Name;
            }
        }

        std::string sig;
        std::vector<uint8_t> id;
        if (funType == Function) {
            sig = rawName + "(" + Join(types, ",") + ")";
            Hash hash = Keccak256(sig);
            std::string hex = hash.Hex();
            id = std::vector<uint8_t>(hex.begin(), hex.end());
            id.resize(4);
        }

        std::string state = mutability;
        if (state == "nonpayable") {
            state = "";
        }
        if (!state.empty()) {
            state += " ";
        }
        std::string identity;
        if (funType == Fallback) {
            identity = "fallback";
        } else if (funType == Receive) {
            identity = "receive";
        } else if (funType == Constructor) {
            identity = "constructor";
        } else {
            identity = "function " + rawName;
        }

        str = identity + "(" + Join(inputNames, ", ") + ") " + state + "returns(" + Join(outputNames, ", ") + ")";

        Name = name;
        RawName = rawName;
        Type = funType;
        StateMutability = mutability;
        Constant = isConst;
        Payable = isPayable;
        Inputs = inputs;
        Outputs = outputs;
        str = str;
        Sig = sig;
        ID = id;
    }

    std::string String() const {
        return str;
    }

    bool IsConstant() const {
        return StateMutability == "view" || StateMutability == "pure" || Constant;
    }

    bool IsPayable() const {
        return StateMutability == "payable" || Payable;
    }

private:
    std::string Join(const std::vector<std::string>& elements, const std::string& delimiter) const {
        std::stringstream ss;
        std::copy(elements.begin(), elements.end(), std::ostream_iterator<std::string>(ss, delimiter.c_str()));
        std::string result = ss.str();
        return result.substr(0, result.length() - delimiter.length());
    }
};