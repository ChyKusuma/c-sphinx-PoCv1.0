#include <string>
#include <functional>

namespace abi {
    // ResolveNameConflict returns the next available name for a given thing.
    // This helper can be used for lots of purposes:
    //
    // - In solidity function overloading is supported, this function can fix
    //   the name conflicts of overloaded functions.
    // - In golang binding generation, the parameter(in function, event, error,
    //	 and struct definition) name will be converted to camelcase style which
    //	 may eventually lead to name conflicts.
    //
    // Name conflicts are mostly resolved by adding number suffix.
    // 	 e.g. if the abi contains Methods send, send1
    //   ResolveNameConflict would return send2 for input send.
    std::string ResolveNameConflict(const std::string& rawName, std::function<bool(const std::string&)> used) {
        std::string name = rawName;
        bool ok = used(name);
        for (int idx = 0; ok; ++idx) {
            name = rawName + std::to_string(idx);
            ok = used(name);
        }
        return name;
    }
}