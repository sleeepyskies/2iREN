#pragma once

#include "2iren/util/identifier.hpp"
#include "2iren/util/type_info.hpp"
#include "asset.hpp"


namespace siren::asset {

/**
 * @brief Identifier for assets. Contains additional data such as a unique id, an index,
 * as well as type information.
 */
struct AssetID final : Identifier<AssetID> {
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
    AssetID(
        const IndexType index,
        const GenerationType gen,
        const TypeID type
    ) : Identifier(index, gen, type) { }

    AssetID(const AssetID&)            = default;
    AssetID& operator=(const AssetID&) = default;
    AssetID(AssetID&&)                 = default;
    AssetID& operator=(AssetID&&)      = default;

    /// todo: does this break since we are casting 64-bit to 16-bit?
    /** @brief Returns a 16-bit TypeID for the Asset of type A. */
    template <IsAsset A>
    [[nodiscard]]
    static constexpr TypeID get_type_id() noexcept { return static_cast<TypeID>(typehash_of<A>()); }

    /** @brief Returns the TypeID of this AssetID. */
    [[nodiscard]] constexpr auto type() const noexcept -> TypeID { return meta(); }

    static constexpr auto invalid() noexcept -> AssetID { return AssetID{ }; }
};

} // namespace siren::asset
