#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <tuple>

class ArgumentMarshaling {
public:
    ArgumentMarshaling(const std::string& name, const std::string& type, const std::string& internalType,
                       const std::vector<ArgumentMarshaling>& components, bool indexed)
        : Name(name), Type(type), InternalType(internalType), Components(components), Indexed(indexed) {}
    std::string Name;
    std::string Type;
    std::string InternalType;
    std::vector<ArgumentMarshaling> Components;
    bool Indexed;
};

class SelectorMarshaling {
public:
    SelectorMarshaling(const std::string& name, const std::string& type, const std::vector<ArgumentMarshaling>& inputs)
        : Name(name), Type(type), Inputs(inputs) {}
    std::string Name;
    std::string Type;
    std::vector<ArgumentMarshaling> Inputs;
};

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isIdentifierSymbol(char c) {
    return c == '$' || c == '_';
}

std::tuple<std::string, std::string, std::string> parseToken(const std::string& unescapedSelector, bool isIdent) {
    if (unescapedSelector.empty()) {
        throw std::invalid_argument("empty token");
    }
    char firstChar = unescapedSelector[0];
    size_t position = 1;
    if (!(isAlpha(firstChar) || (isIdent && isIdentifierSymbol(firstChar)))) {
        throw std::invalid_argument("invalid token start: " + std::string(1, firstChar));
    }
    while (position < unescapedSelector.size()) {
        char ch = unescapedSelector[position];
        if (!(isAlpha(ch) || isDigit(ch) || (isIdent && isIdentifierSymbol(ch)))) {
            break;
        }
        position++;
    }
    return std::make_tuple(unescapedSelector.substr(0, position), unescapedSelector.substr(position), "");
}

std::tuple<std::string, std::string, std::string> parseIdentifier(const std::string& unescapedSelector) {
    return parseToken(unescapedSelector, true);
}

std::tuple<std::string, std::string, std::string> parseElementaryType(const std::string& unescapedSelector) {
    auto [parsedType, rest, _] = parseToken(unescapedSelector, false);
    // handle arrays
    while (!rest.empty() && rest[0] == '[') {
        parsedType += rest[0];
        rest = rest.substr(1);
        while (!rest.empty() && isDigit(rest[0])) {
            parsedType += rest[0];
            rest = rest.substr(1);
        }
        if (rest.empty() || rest[0] != ']') {
            throw std::invalid_argument("failed to parse array: expected ']', got " + rest.substr(0, 1));
        }
        parsedType += rest[0];
        rest = rest.substr(1);
    }
    return std::make_tuple(parsedType, rest, "");
}

std::tuple<std::vector<std::string>, std::string> parseCompositeType(const std::string& unescapedSelector) {
    if (unescapedSelector.empty() || unescapedSelector[0] != '(') {
        throw std::invalid_argument("expected '(', got " + std::string(1, unescapedSelector[0]));
    }
    auto [parsedType, rest, _] = parseType(unescapedSelector.substr(1));
    std::vector<std::string> result = {parsedType};
    while (!rest.empty() && rest[0] != ')') {
        auto [parsedType, newRest, _] = parseType(rest.substr(1));
        result.push_back(parsedType);
        rest = newRest;
    }
    if (rest.empty() || rest[0] != ')') {
        throw std::invalid_argument("expected ')', got '" + rest + "'");
    }
    return std::make_tuple(result, rest.substr(1));
}

std::tuple<std::string, std::string, std::string> parseType(const std::string& unescapedSelector) {
    if (unescapedSelector.empty()) {
        throw std::invalid_argument("empty type");
    }
    if (unescapedSelector[0] == '(') {
        auto [parsedType, rest, _] = parseCompositeType(unescapedSelector);
        return std::make_tuple(parsedType.front(), rest, "");
    } else {
        return parseElementaryType(unescapedSelector);
    }
}

std::vector<ArgumentMarshaling> assembleArgs(const std::vector<std::string>& args) {
    std::vector<ArgumentMarshaling> arguments;
    for (size_t i = 0; i < args.size(); ++i) {
        std::string name = "name" + std::to_string(i);
        arguments.emplace_back(name, args[i], args[i], std::vector<ArgumentMarshaling>(), false);
    }
    return arguments;
}

SelectorMarshaling ParseSelector(const std::string& unescapedSelector) {
    auto [name, rest, _] = parseIdentifier(unescapedSelector);
    std::vector<std::string> args;
    if (rest.size() >= 2 && rest[0] == '(' && rest[1] == ')') {
        rest = rest.substr(2);
    } else {
        auto [parsedType, newRest, _] = parseCompositeType(rest);
        args = parsedType;
        rest = newRest;
    }
    if (!rest.empty()) {
        throw std::invalid_argument("unexpected string '" + rest + "'");
    }

    // Reassemble the fake ABI and construct the JSON
    std::vector<ArgumentMarshaling> fakeArgs = assembleArgs(args);

    return SelectorMarshaling(name, "function", fakeArgs);
}
