#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include <bytes.hpp>
#include <encoding/json.hpp> //Required for JSON parsing
#include <errors.hpp>
#include <fmt.hpp>
#include <io.hpp>

#include <common.hpp>
#include <crypto.hpp>

namespace abi {
    enum MethodType {
        Constructor,
        Function,
        Fallback,
        Receive
    };

    // Forward declarations
    struct Argument;
    struct Method;

    struct Event {
        std::string Name;
        bool Anonymous;
        std::vector<Argument> Inputs;
    };

    struct Error {
        std::string Name;
        std::vector<Argument> Inputs;
    };

    struct Argument {
        std::string Type;
        std::string Name;
    };

    struct Method {
        std::string Name;
        MethodType Type;
        std::string StateMutability;
        bool Constant;
        bool Payable;
        std::vector<Argument> Inputs;
        std::vector<Argument> Outputs;
        std::vector<uint8_t> ID;
    };

    typedef std::vector<Argument> Arguments;

    class ABI {
    public:
        Method Constructor;
        std::map<std::string, Method> Methods;
        std::map<std::string, Event> Events;
        std::map<std::string, Error> Errors;
        Method Fallback;
        Method Receive;

        std::vector<uint8_t> Pack(const std::string& name, const std::vector<std::string>& args) {
            std::vector<uint8_t> packedData;

            // Encode the function selector
            std::vector<uint8_t> selector = GetFunctionSelector(name);
            packedData.insert(packedData.end(), selector.begin(), selector.end());

            // Encode each argument
            for (const auto& arg : args) {
                std::vector<uint8_t> encodedArg = EncodeArgument(arg);
                packedData.insert(packedData.end(), encodedArg.begin(), encodedArg.end());
            }

            return packedData;
        }

        std::vector<std::string> Unpack(const std::string& name, const std::vector<uint8_t>& data) {
            std::vector<std::string> unpackedArgs;

            // Decode the function selector and remove it from the data
            std::vector<uint8_t> selector(data.begin(), data.begin() + 4);
            std::string functionName = GetFunctionName(selector); // Convert selector to function name
            if (functionName != name) {
                // Function name mismatch
                // Handle error or return empty vector
                return unpackedArgs;
            }

            // Decode each argument
            size_t offset = 4; // Start after the function selector
            for (size_t i = 0; i < argsCount; ++i) {
                std::string decodedArg = DecodeArgument(data, offset);
                unpackedArgs.push_back(decodedArg);
                // Update offset for the next argument
                offset += decodedArg.size(); // Update offset based on the size of the decoded argument
            }

            return unpackedArgs;
        }

        void UnpackIntoInterface(void* v, const std::string& name, const std::vector<uint8_t>& data) {
            // Cast the void pointer to the interface structure type
            Interface* interface = static_cast<Interface*>(v);

            // Decode the function selector and remove it from the data
            std::vector<uint8_t> selector(data.begin(), data.begin() + 4);
            std::string functionName = GetFunctionName(selector); // Convert selector to function name
            if (functionName != name) {
                // Function name mismatch
                // Handle error or return
                return;
            }

            // Decode each argument
            size_t offset = 4; // Start after the function selector
            for (size_t i = 0; i < interface->Inputs.size(); ++i) {
                std::string decodedArg = DecodeArgument(data, offset);
                interface->Inputs[i] = decodedArg;
                // Update offset for the next argument
                offset += decodedArg.size(); // Update offset based on the size of the decoded argument
            }

            // Decode each output
            for (size_t i = 0; i < interface->Outputs.size(); ++i) {
                std::string decodedArg = DecodeArgument(data, offset);
                interface->Outputs[i] = decodedArg;
                // Update offset for the next argument
                offset += decodedArg.size(); // Update offset based on the size of the decoded argument
            }
        }

        std::map<std::string, std::string> UnpackIntoMap(const std::string& name, const std::vector<uint8_t>& data) {
            std::map<std::string, std::string> unpackedMap;

            // Decode the function selector and remove it from the data
            std::vector<uint8_t> selector(data.begin(), data.begin() + 4);
            std::string functionName = GetFunctionName(selector); // Convert selector to function name
            if (functionName != name) {
                // Function name mismatch
                // Handle error or return empty map
                return unpackedMap;
            }

            // Decode each argument
            size_t offset = 4; // Start after the function selector
            for (size_t i = 0; i < argsCount; ++i) {
                std::string argName = GetArgumentName(i); // Get argument name based on index
                std::string decodedArg = DecodeArgument(data, offset);
                unpackedMap[argName] = decodedArg;
                // Update offset for the next argument
                offset += decodedArg.size(); // Update offset based on the size of the decoded argument
            }

            return unpackedMap;
        }

        void UnmarshalJSON(const std::string& data) {
            // Parse the JSON string
            nlohmann::json json_data = nlohmann::json::parse(data);

            // Extract Constructor data
            if (json_data.find("Constructor") != json_data.end()) {
                Constructor = ParseMethod(json_data["Constructor"]);
            }

            // Extract Methods data
            if (json_data.find("Methods") != json_data.end()) {
                for (const auto& method : json_data["Methods"].items()) {
                    Methods[method.key()] = ParseMethod(method.value());
                }
            }

            // Extract Events data
            if (json_data.find("Events") != json_data.end()) {
                for (const auto& event : json_data["Events"].items()) {
                    Events[event.key()] = ParseEvent(event.value());
                }
            }

            // Extract Errors data
            if (json_data.find("Errors") != json_data.end()) {
                for (const auto& error : json_data["Errors"].items()) {
                    Errors[error.key()] = ParseError(error.value());
                }
            }

            // Extract Fallback data
            if (json_data.find("Fallback") != json_data.end()) {
                Fallback = ParseMethod(json_data["Fallback"]);
            }

            // Extract Receive data
            if (json_data.find("Receive") != json_data.end()) {
                Receive = ParseMethod(json_data["Receive"]);
            }
        }

        Method ParseMethod(const nlohmann::json& method_data) {
            Method method;
            method.Name = method_data["Name"];
            method.Type = ParseMethodType(method_data["Type"]);
            method.StateMutability = method_data["StateMutability"];
            method.Constant = method_data["Constant"];
            method.Payable = method_data["Payable"];
            method.Inputs = ParseArguments(method_data["Inputs"]);
            method.Outputs = ParseArguments(method_data["Outputs"]);
            // Assuming ID is represented as string in JSON and needs to be converted to vector<uint8_t>
            method.ID = ParseID(method_data["ID"]);
            return method;
        }

        Event ParseEvent(const nlohmann::json& event_data) {
            Event event;
            event.Name = event_data["Name"];
            event.Anonymous = event_data["Anonymous"];
            event.Inputs = ParseArguments(event_data["Inputs"]);
            return event;
        }

        Error ParseError(const nlohmann::json& error_data) {
            Error error;
            error.Name = error_data["Name"];
            error.Inputs = ParseArguments(error_data["Inputs"]);
            return error;
        }

        MethodType ParseMethodType(const std::string& type_str) {
            // Implement logic to parse string to MethodType enum
            // Example implementation:
            if (type_str == "Constructor") {
                return Constructor;
            } else if (type_str == "Function") {
                return Function;
            } else if (type_str == "Fallback") {
                return Fallback;
            } else if (type_str == "Receive") {
                return Receive;
            }
        }

        std::vector<Argument> ParseArguments(const nlohmann::json& arguments_data) {
            std::vector<Argument> arguments;
            for (const auto& arg : arguments_data) {
                Argument argument;
                argument.Type = arg["Type"];
                argument.Name = arg["Name"];
                arguments.push_back(argument);
            }
            return arguments;
        }

        std::vector<uint8_t> ParseID(const std::string& id_str) {
            // Implement logic to parse string to vector<uint8_t>
            // Example implementation:
            std::vector<uint8_t> id;
            for (const auto& c : id_str) {
                id.push_back(static_cast<uint8_t>(c));
            }
            return id;
        }


        Method* MethodById(const std::vector<uint8_t>& sigdata) {
            // Iterate over each method to find the one with matching ID
            for (auto& methodPair : Methods) {
                Method& method = methodPair.second;
                // Check if the method's ID matches the given sigdata
                if (method.ID == sigdata) {
                    return &method;
                }
            }
            // Method with the given ID not found
            return nullptr;
        }


        Event* EventByID(const std::vector<uint8_t>& topic) {
            // Iterate over each event to find the one with matching topic
            for (auto& eventPair : Events) {
                Event& event = eventPair.second;
                // Check if the event's ID matches the given topic
                if (event.ID == topic) {
                    return &event;
                }
            }
            // Event with the given ID not found
            return nullptr;
        }

        bool HasFallback() {
            return Fallback.Type == Fallback;
        }

        bool HasReceive() {
            return Receive.Type == Receive;
        }

        static std::string UnpackRevert(const std::vector<uint8_t>& data) {
            // Convert byte data to string
            std::string revertReason(data.begin(), data.end());
            return revertReason;
        }
    };

    ABI JSON(const std::string& reader) {
        ABI abi;

        // Parse the JSON string
        nlohmann::json json_data = nlohmann::json::parse(reader);

        // Extract Constructor data
        if (json_data.find("Constructor") != json_data.end()) {
            abi.Constructor = ParseMethod(json_data["Constructor"]);
        }

        // Extract Methods data
        if (json_data.find("Methods") != json_data.end()) {
            for (const auto& method : json_data["Methods"].items()) {
                abi.Methods[method.key()] = ParseMethod(method.value());
            }
        }

        // Extract Events data
        if (json_data.find("Events") != json_data.end()) {
            for (const auto& event : json_data["Events"].items()) {
                abi.Events[event.key()] = ParseEvent(event.value());
            }
        }

        // Extract Errors data
        if (json_data.find("Errors") != json_data.end()) {
            for (const auto& error : json_data["Errors"].items()) {
                abi.Errors[error.key()] = ParseError(error.value());
            }
        }

        // Extract Fallback data
        if (json_data.find("Fallback") != json_data.end()) {
            abi.Fallback = ParseMethod(json_data["Fallback"]);
        }

        // Extract Receive data
        if (json_data.find("Receive") != json_data.end()) {
            abi.Receive = ParseMethod(json_data["Receive"]);
        }

        return abi;
    }

    std::string ResolveNameConflict(const std::string& name, const std::function<bool(const std::string&)>& predicate) {
        std::string resolvedName = name;
        int suffix = 1;
        // Keep incrementing suffix until there's no conflict
        while (predicate(resolvedName)) {
            resolvedName = name + std::to_string(suffix);
            suffix++;
        }
        return resolvedName;
    }
}
