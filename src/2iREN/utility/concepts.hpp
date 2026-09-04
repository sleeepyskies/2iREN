#pragma once

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

#include "2iREN/base.hpp"

namespace siren {

/// @brief Ensures that the type T is some numerical type.
template <typename T>
concept IsArithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept IsComparable = std::three_way_comparable<T>;

/**
 * @brief Ensures some type T is a reference.
 * @tparam T The type to check.
 */
template <typename T>
concept IsReference = std::is_reference_v<T>;

/**
 * @brief Ensures a type is an enum.
 * @tparam E The type to check.
 */
template <typename E>
concept IsEnum = std::is_enum_v<E>;

/**
 * @brief Ensures a type is an enum with a Max member for sizing.
 * @tparam E The type to check.
 * @note There is no guarantee that Max is actually the size of the enum,
 * it is just a convention.
 */
template <typename E>
concept IsSizedEnum = std::is_enum_v<E> && requires {
    { std::to_underlying(E::Max) } -> std::convertible_to<usize>;
};

/**
 * @brief Ensures the give type is callable.
 * @tparam F The type to check.
 */
template <typename F>
concept IsCallable = std::is_invocable_v<F>;

/**
 * @brief Ensures a function is a predicate.
 * @tparam F The function to check.
 */
template <typename F>
concept IsPredicate =
    std::is_invocable_v<F> && std::is_convertible_v<decltype(std::declval<F>()()), bool>;

/**
 * @brief Ensures a type is copyable.
 * @tparam T The type to check
 */
template <typename T>
concept IsCopyable = std::is_trivially_copyable_v<T>;

/**
 * @brief Ensures a type is default constructible.
 * @tparam T The type to check
 */
template <typename T>
concept HasDefaultConstructor = std::is_default_constructible_v<T>;

/// @brief Ensures that @tparam From can be converted to @tparam To.
template <typename From, typename To>
concept CanConvertTo = std::convertible_to<From, To>;

} // namespace siren
