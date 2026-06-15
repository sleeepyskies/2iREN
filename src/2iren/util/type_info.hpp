#pragma once

#include <string_view>

#include "hashed_string.hpp"

#if defined __clang__ || defined __GNUC__
#   define SIREN_PRETTY_FUNCTION __PRETTY_FUNCTION__
#   define SIREN_PRETTY_FUNCTION_PREFIX '='
#   define SIREN_PRETTY_FUNCTION_SUFFIX ']'
#elif defined _MSC_VER
#   define SIREN_PRETTY_FUNCTION __FUNCSIG__
#   define SIREN_PRETTY_FUNCTION_PREFIX '<'
#   define SIREN_PRETTY_FUNCTION_SUFFIX '>'
#endif

namespace siren {

template <typename Type>
[[nodiscard]] constexpr const char* pretty_function() noexcept {
#if defined SIREN_PRETTY_FUNCTION
    return SIREN_PRETTY_FUNCTION;
#else
    return "";
#endif
}

template <typename Type>
[[nodiscard]] constexpr std::string_view stripped_type_name() noexcept {
#if defined SIREN_PRETTY_FUNCTION
    const std::string_view full_name{ pretty_function<Type>() };
    const auto first = full_name.find_first_not_of(' ', full_name.find_first_of(SIREN_PRETTY_FUNCTION_PREFIX) + 1);
    return full_name.substr(first, full_name.find_last_of(SIREN_PRETTY_FUNCTION_SUFFIX) - first);
#else
    return std::string_view{ };
#endif
}

template <typename Type, auto = stripped_type_name<Type>().find_first_of('.')>
[[nodiscard]] constexpr std::string_view type_name_helper(int) noexcept {
    return stripped_type_name<Type>();
}

template <typename Type>
[[nodiscard]] std::string_view type_name_helper(char) noexcept {
    static const std::string_view value = stripped_type_name<Type>();
    return value;
}

/**
 * @brief Utility for fetching a human-readable name for a given type.
 * @tparam Type The Type to reflect.
 */
template <typename Type>
struct TypeName final {
    /**
     * @brief Returns the type name.
     */
    [[nodiscard]] static constexpr auto value() noexcept -> std::string_view {
        return type_name_helper<Type>(0);
    }

    /**
     * @brief Implicit conversion to string_view.
     */
    [[nodiscard]] constexpr operator std::string_view() const noexcept {
        return type_name_helper<Type>(0);
    }
};

/**
 * @brief Utility for providing a type-unique hash for a given type.
 * @tparam Type The type to reflect.
 */
template <typename Type>
struct TypeHash final {
    /** @brief Returns the type hash. */
    [[nodiscard]] static constexpr auto value() noexcept -> HashedString {
        return HashedString{ type_name_helper<Type>(0).data() };
    }

    /** @brief Implicit conversion to @ref HashedString. */
    [[nodiscard]] constexpr operator HashedString() const noexcept {
        return HashedString{ type_name_helper<Type>(0).data() };
    }

    /** @brief Shorthand for fetching the hash value of the @ref HashedString. */
    [[nodiscard]] static constexpr auto hash() noexcept -> HashedString::HashType {
        return value().hash();
    }
};

/**
 * @brief Returns the cleaned name of a type.
 * @tparam T The type to find the name of.
 */
template <typename T>
constexpr auto typename_of() -> std::string_view {
    return TypeName<T>::value();
}

/**
 * @brief Returns the hash of a type.
 * @tparam T The type to find the type hash of.
 */
template <typename T>
constexpr auto typehash_of() -> HashedString::HashType {
    return TypeHash<T>::hash();
}

} // namespace siren::refl
