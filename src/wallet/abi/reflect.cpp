#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <map>

// Dummy implementation of strings.ToLower for demonstration purposes
std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Dummy implementation of strings.Title for demonstration purposes
std::string TitleCase(const std::string& str) {
    std::string result = str;
    if (!result.empty()) {
        result[0] = toupper(result[0]);
    }
    return result;
}

// Dummy implementation of set for demonstration purposes
void set(reflect::Value dst, reflect::Value src) {
    // Implementation here
}

// Dummy implementation of reflect::Value for demonstration purposes
class Value {
public:
    template<typename T>
    Value(T val) : value(val) {}
    template<typename T>
    T get() const { return value; }
private:
    void* value;
};

// Dummy implementation of reflect::Type for demonstration purposes
class Type {
public:
    Type(std::string name) : Name(name) {}
    std::string Name;
};

// Dummy implementation of abi::ToLower for demonstration purposes
std::string ToCamelCase(const std::string& str) {
    std::string result;
    bool capitalizeNext = true;
    for (char c : str) {
        if (c == '_') {
            capitalizeNext = true;
        } else {
            if (capitalizeNext) {
                result.push_back(toupper(c));
                capitalizeNext = false;
            } else {
                result.push_back(c);
            }
        }
    }
    return result;
}

// ConvertType converts an interface of a runtime type into a interface of the
// given type
template<typename T>
T ConvertType(const T& in, const T& proto) {
    Type protoType(proto);
    if (typeid(in).name() == typeid(protoType).name()) {
        return in;
    }
    // Use set as a last ditch effort
    set(proto, in);
    return proto;
}

// mapArgNamesToStructFields maps a slice of argument names to struct fields.
// first round: for each Exportable field that contains a `abi:""` tag
//   and this field name exists in the given argument name list, pair them together.
// second round: for each argument name that has not been already linked,
//   find what variable is expected to be mapped into, if it exists and has not been
//   used, pair them.
// Note this function assumes the given value is a struct value.
std::map<std::string, std::string> mapArgNamesToStructFields(const std::vector<std::string>& argNames, const reflect::Value& value) {
    std::map<std::string, std::string> abi2struct;
    std::map<std::string, std::string> struct2abi;

    // Implementation here

    return abi2struct;
}