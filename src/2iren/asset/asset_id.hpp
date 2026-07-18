#pragma once

#include <format>

#include "2iren/util/identifier.hpp"
#include "2iren/util/type_info.hpp"
#include "fwd.hpp"

namespace siren {

/**
 * @brief Identifier for assets. Contains additional data such as a unique id, an index,
 * as well as type information.
 */
struct AssetId final : Identifier<AssetId> {
    using Identifier::Identifier;

    using GenerationType = Identifier::GenerationType;
    using IndexType      = Identifier::IndexType;
    using TypeID         = Identifier::Meta;

    /**
     * @brief Constructs a new AssetID.
     * @param index The value to use for indexing this identifier.
     * @param gen The generation of this identifier.
     * @param type The type of this identifier.
     */
    AssetId(const IndexType index, const GenerationType gen, const TypeID type) : Identifier(index, gen, type) {}

    /** @brief Constructs a new invalid AssetID. */
    static constexpr auto invalid() noexcept -> AssetId { return AssetId{}; }

    AssetId(const AssetId&)            = default;
    AssetId& operator=(const AssetId&) = default;
    AssetId(AssetId&&)                 = default;
    AssetId& operator=(AssetId&&)      = default;

    /// todo: does this break since we are casting 64-bit to 16-bit?
    /** @brief Returns a 16-bit TypeID for the Asset of type A. */
    template <IsAsset A>
    [[nodiscard]] static constexpr auto type_id() noexcept -> TypeID {
        return static_cast<TypeID>(typehash_of<A>());
    }

    /** @brief Returns the TypeID of this AssetID. */
    [[nodiscard]] constexpr auto type() const noexcept -> TypeID { return meta(); }

    [[nodiscard]] constexpr auto to_string() const -> std::string {
        return std::format("AssetId(index={}, generation={}, type={})", index(), generation(), type());
    }

private:
    AssetId() = default;
};

} // namespace siren
