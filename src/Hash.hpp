// Copyright (c) [2023] SPHINX-HUB
// All rights reserved.
// This software is distributed under the MIT License.

#ifndef HASH_HPP
#define HASH_HPP

#include "Crypto/Swifftx/SHA3.h"
#include "Crypto/Swifftx/SWIFFTX.h"

namespace SPHINXHash {

    // Define the Hash() function interface
    HashReturn Hash(int hashbitlen, const BitSequence *data, DataLength databitlen, 
                    BitSequence *hashval);

    // Define the existing hash function interface
    std::string SPHINX_256(const std::string& message);

} // namespace SPHINXHash

#endif // HASH_HPP
