#include <vector>
#include <string>
#include <array>
#include <map>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>

constexpr size_t CommonHashLength = 32;
constexpr size_t CommonAddressLength = 20;

class common {
public:
    static constexpr size_t HashLength = CommonHashLength;
    static constexpr size_t AddressLength = CommonAddressLength;

    class Hash : public std::array<unsigned char, HashLength> {
    public:
        using std::array<unsigned char, HashLength>::array;
    };

    class Address : public std::array<unsigned char, AddressLength> {
    public:
        using std::array<unsigned char, AddressLength>::array;
    };

    static void copyHash(const Hash& src, Hash& dst) {
        std::copy(src.begin(), src.end(), dst.begin());
    }
};

class crypto {
public:
    static common::Hash Keccak256Hash(const std::string& input) {
        // Dummy implementation, replace with actual hashing algorithm
        common::Hash hash;
        for (size_t i = 0; i < input.size() && i < common::HashLength; ++i) {
            hash[i] = input[i];
        }
        return hash;
    }

    static common::Hash Keccak256Hash(const std::vector<unsigned char>& input) {
        // Dummy implementation, replace with actual hashing algorithm
        common::Hash hash;
        for (size_t i = 0; i < input.size() && i < common::HashLength; ++i) {
            hash[i] = input[i];
        }
        return hash;
    }
};

template<typename T>
std::string toHexString(const T& data) {
    std::ostringstream oss;
    for (const auto& byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

class Arguments {
public:
    // Implement Arguments class according to your needs
};

// MakeTopics converts a filter query argument list into a filter topic set.
std::vector<std::vector<common::Hash>> MakeTopics(const std::vector<std::vector<Arguments>>& query) {
    std::vector<std::vector<common::Hash>> topics(query.size());
    for (size_t i = 0; i < query.size(); ++i) {
        for (const auto& filter : query[i]) {
            for (const auto& rule : filter) {
                common::Hash topic;

                // Try to generate the topic based on simple types
                if (rule.template is<common::Hash>()) {
                    common::copyHash(rule.template get<common::Hash>(), topic);
                } else if (rule.template is<common::Address>()) {
                    const auto& address = rule.template get<common::Address>();
                    std::copy(address.begin(), address.end(), topic.begin() + (common::HashLength - common::AddressLength));
                } else if (rule.template is<std::shared_ptr<bigInt>>) {
                    auto bigInt = rule.template get<std::shared_ptr<bigInt>>();
                    auto blob = bigInt->toBytes();
                    std::copy(blob.begin(), blob.end(), topic.begin() + (common::HashLength - blob.size()));
                } else if (rule.template is<bool>()) {
                    auto value = rule.template get<bool>();
                    topic[common::HashLength - 1] = value ? 1 : 0;
                } else if (rule.template is<int8_t>() || rule.template is<int16_t>() || rule.template is<int32_t>() || rule.template is<int64_t>()) {
                    auto value = rule.template get<int64_t>();
                    std::copy(genIntType(value, sizeof(value)).begin(), genIntType(value, sizeof(value)).end(), topic.begin());
                } else if (rule.template is<uint8_t>() || rule.template is<uint16_t>() || rule.template is<uint32_t>() || rule.template is<uint64_t>()) {
                    auto value = rule.template get<uint64_t>();
                    auto blob = bigInt(value).toBytes();
                    std::copy(blob.begin(), blob.end(), topic.begin() + (common::HashLength - blob.size()));
                } else if (rule.template is<std::string>()) {
                    const auto& str = rule.template get<std::string>();
                    auto hash = crypto::Keccak256Hash(str);
                    common::copyHash(hash, topic);
                } else if (rule.template is<std::vector<unsigned char>>()) {
                    const auto& bytes = rule.template get<std::vector<unsigned char>>();
                    auto hash = crypto::Keccak256Hash(bytes);
                    common::copyHash(hash, topic);
                } else {
                    // Attempt to generate the topic from funky types
                    // Note: Dynamic types cannot be reconstructed since they get mapped to Keccak256
                    // hashes as the topic value!
                    throw std::runtime_error("Unsupported indexed type");
                }
                topics[i].push_back(topic);
            }
        }
    }
    return topics;
}

std::vector<unsigned char> genIntType(int64_t rule, size_t size) {
    std::vector<unsigned char> topic(common::HashLength, 0);
    if (rule < 0) {
        // if a rule is negative, we need to put it into two's complement.
        // extended to common.HashLength bytes.
        std::fill(topic.begin(), topic.end(), 0xFF);
    }
    for (size_t i = 0; i < size; i++) {
        topic[common::HashLength - i - 1] = static_cast<unsigned char>(rule >> (i * 8));
    }
    return topic;
}

// ParseTopics converts the indexed topic fields into actual log field values.
template<typename T>
void ParseTopics(T& out, const Arguments& fields, const std::vector<common::Hash>& topics) {
    parseTopicWithSetter(out, fields, topics,
        [&](const auto& arg, const auto& reconstr) {
            auto field = out.getFieldByName(toCamelCase(arg.Name));
            field.set(reconstr);
        });
}

// ParseTopicsIntoMap converts the indexed topic field-value pairs into map key-value pairs.
void ParseTopicsIntoMap(std::map<std::string, unsigned char>& out, const Arguments& fields, const std::vector<common::Hash>& topics) {
    parseTopicWithSetter(out, fields, topics,
        [&](const auto& arg, const auto& reconstr) {
            out[arg.Name] = reconstr;
        });
}

// parseTopicWithSetter converts the indexed topic field-value pairs and stores them using the
// provided set function.
template<typename T, typename Setter>
void parseTopicWithSetter(T& out, const Arguments& fields, const std::vector<common::Hash>& topics, const Setter& setter) {
    // Sanity check that the fields and topics match up
    if (fields.size() != topics.size()) {
        throw std::runtime_error("Topic/field count mismatch");
    }
    // Iterate over all the fields and reconstruct them from topics
    for (size_t i = 0; i < fields.size(); ++i) {
        auto& arg = fields[i];
        if (!arg.Indexed) {
            throw std::runtime_error("Non-indexed field in topic reconstruction");
        }
        auto& topic = topics[i];
        auto& reconstr = [&](){
            if (arg.Type.T == TupleTy) {
                throw std::runtime_error("Tuple type in topic reconstruction");
            } else if (arg.Type.T == StringTy || arg.Type.T == BytesTy || arg.Type.T == SliceTy || arg.Type.T == ArrayTy) {
                // Array types (including strings and bytes) have their keccak256 hashes stored in the topic- not a hash
                // whose bytes can be decoded to the actual value- so the best we can do is retrieve that hash
                return topic;
            } else if (arg.Type.T == FunctionTy) {
                if (binary::BigEndian::load64(topic.data()) != 0) {
                    throw std::runtime_error("Bind: got improperly encoded function type");
                }
                return std::array<unsigned char, 24>(topic.begin() + 8, topic.begin() + 32);
            } else {
                // Implement toGoType function according to your needs
                return toGoType(arg.Type, topic);
            }
        }();
        // Use the setter function to store the value
        setter(arg, reconstr);
    }
}