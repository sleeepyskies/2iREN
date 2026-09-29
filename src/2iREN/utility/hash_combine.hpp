#pragma once

#include <functional>

#include "2iREN/core/base.hpp"

namespace siren {

/// @brief Takes in a hash value and a non hashed value as input, hashes the 
/// non hashed value and combines the existing has with the new hash.
/// source: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
template <typename T>
constexpr auto hash_combine(usize& old, const T& value) -> void {
    const auto hasher = std::hash<T>{};
    old ^= hasher(value) + 0x9e3779b9 + (old << 6) + (old >> 2);
}

} // namespace siren
