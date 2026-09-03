#pragma once

#include <format>
#include <limits>

#include "2iREN/base.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren {

namespace impl {
struct NullHandle_t {
    template <typename Handle>
    constexpr operator Handle() const {
        return Handle::invalid();
    }
};
} // namespace impl

inline constexpr auto NullHandle = impl::NullHandle_t{};

/**
 * @struct Identifier
 * @brief A packed integer handle. Provides unique identification
 * and can prevent stale references. Also allows for additional meta-data
 * for the use.
 * @tparam T The derived CRTP type.
 * @note Bit layout (MSB to LSB) (assuming 64-bit handle):
 * - [32-63] Index: Lookup key.
 * - [16-31] Generation: Should be incremented each time a slot is reused to avoid invalid
 * references.
 * - [00-15] Meta: Some user defined extra meta-data.
 * @todo: support for multiple idtype sizes aka u64 and u32 and u16 maybe even idk, then resource
 * can be just u32
 */
template <typename T>
class Identifier {
protected:
    /** @brief The main storage type of the packed id. */
    using IdType = u64;
    /** @brief The type used for indexing. */
    using IndexType = u32;
    /** @brief The type used for the generation of the id. */
    using GenerationType = u16;
    /** @brief The type used for custom additional data. */
    using Meta = u16;

    static constexpr IdType INVALID_ID = std::numeric_limits<IdType>::max();

    /** @brief The full packed ID. */
    IdType m_id = INVALID_ID;

    constexpr Identifier() noexcept = default;
    constexpr Identifier(const IndexType idx, const GenerationType gen, const Meta meta) noexcept :
        m_id(pack(idx, gen, meta)) {}

public:
    Identifier(const Identifier&)            = default;
    Identifier& operator=(const Identifier&) = default;
    Identifier(Identifier&&)                 = default;
    Identifier& operator=(Identifier&&)      = default;

    /** @brief Returns the full packed value of this id. */
    [[nodiscard]] constexpr auto packed() const noexcept -> IdType { return m_id; }

    /** @brief Returns the index of this id. */
    [[nodiscard]] constexpr auto index() const noexcept -> IndexType {
        return static_cast<IndexType>((m_id >> 32) & 0xFFFFFFFF);
    }
    /** @brief Returns the generation of this id. */
    [[nodiscard]] constexpr auto generation() const noexcept -> GenerationType {
        return static_cast<GenerationType>((m_id >> 16) & 0xFFFF);
    }
    /** @brief Returns the meta of this id. */
    [[nodiscard]] constexpr auto meta() const noexcept -> Meta {
        return static_cast<Meta>(m_id & 0xFFFF);
    }

    /** @brief Simple factory method to return an invalid Identifier. */
    [[nodiscard]] constexpr static auto invalid() noexcept -> Identifier { return T{}; }

    /** @brief Checks if the handle is valid aka has a non 0 inner value. */
    [[nodiscard]] constexpr auto is_valid() const noexcept -> bool { return m_id != INVALID_ID; }
    /** @brief Kills the handle. */
    constexpr auto invalidate() noexcept -> void { m_id = INVALID_ID; }

    /** @brief Returns a hash value for the identifier. Simply uses the full 64-bit integer. */
    [[nodiscard]] constexpr auto hash() const noexcept -> usize { return packed(); }

    /** @brief Equality comparison based on the inner 64-bit value. */
    [[nodiscard]] friend bool operator==(const Identifier&, const Identifier&) = default;

    /** @brief Default to_string implementation for all Identifiers. */
    constexpr auto to_string() const noexcept -> std::string {
        return std::format("Identifier<{}>({})", typename_of<T>(), packed());
    }

private:
    static constexpr auto pack(const IndexType idx, const GenerationType gen, const Meta meta)
        -> IdType {
        IdType id = 0;
        id += static_cast<IdType>(idx) << 32;
        id += static_cast<IdType>(gen) << 16;
        id += static_cast<IdType>(meta);
        return id;
    }
};

/**
 * @brief Type that can be cast to any type of @ref Identifier64 used to represent
 * the null state.
 */
struct NullIdentifier_t {
    template <typename T>
    constexpr operator Identifier<T>() const noexcept {
        return Identifier<T>::invalid();
    }
};

/** @brief Constant used to express any null value for @ref Identifier64. */
inline NullIdentifier_t NullID{};

} // namespace siren

template <typename T>
struct std::hash<siren::Identifier<T>> {
    auto operator()(const siren::Identifier<T>& id) const noexcept -> siren::usize {
        return id.hash();
    }
};
