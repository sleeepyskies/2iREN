#pragma once

#include <compare>
#include <string_view>
#include "2iren/base.hpp"

// Magic numbers used for 64-bit FNV-1a hashing.
constexpr auto PRIME  = 1099511628211ull;
constexpr auto OFFSET = 14695981039346656037ull;


namespace siren {

/**
 * @brief A class representing a hashed string.
 * @note This class does not take ownership of the original string, but instead just holds a
 * string_view of it. The called must make sure to not pass in any temporary objects.
 */
class HashedString {
public:
    using HashType = u64;
    using SizeType = usize;

    /**
     * @brief Creates a new hashes_string using the FNV-1a hash algorithm.
     * @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
     * @param string A std::string_view of the string to hash.
     * @return A 64-bit hash of the input string.
     */
    constexpr HashedString(const std::string_view string) noexcept : m_hash(OFFSET), m_data(string) {
        for (auto c : string) {
            m_hash ^= static_cast<HashType>(c);
            m_hash *= PRIME;
        }
    }

    constexpr HashedString() noexcept : m_hash(0), m_data() { }

    HashedString(const HashedString&)            = default;
    HashedString(HashedString&&)                 = default;
    HashedString& operator=(const HashedString&) = default;
    HashedString& operator=(HashedString&&)      = default;

    /** @brief Three-way comparison between two hashed_string's. */
    [[nodiscard]]
    constexpr auto operator<=>(const HashedString& other) const noexcept {
        return m_hash <=> other.m_hash;
    }

    /** @brief Equality comparison. */
    [[nodiscard]]
    constexpr auto operator==(const HashedString& other) const noexcept -> bool {
        return m_hash == other.m_hash;
    }

    /** @brief Returns the underlying value of the HashedString. */
    [[nodiscard]]
    constexpr auto hash() const noexcept { return m_hash; }

    /** @brief Returns the string view of this hash. */
    [[nodiscard]]
    constexpr auto data() const noexcept -> std::string_view { return m_data; }

    /** @brief Checks if this HashedString has been initialized. */
    [[nodiscard]]
    explicit operator bool() const noexcept { return m_hash != 0; }

private:
    /** @brief The computed hash. */
    HashType m_hash;
    /** @brief View into the original string. */
    std::string_view m_data;
};

} // namespace siren
template <>
struct std::hash<siren::HashedString> {
    auto operator()(const siren::HashedString& hashed_string) const noexcept -> siren::usize {
        return hashed_string.hash();
    }
};
