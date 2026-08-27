#pragma once

#include <libassert/assert.hpp>
#include <format>

#include "fwd.hpp"
#include "asset_id.hpp"
#include "asset_handle.hpp"
#include "asset_pool.hpp"


namespace siren {

/**
 * Creates a new @ref WeakHandle from a @ref StrongHandle.
 */
template <IsAsset A>
[[nodiscard]] auto make_weak(const StrongHandle<A>& strong) noexcept -> WeakHandle {
    return strong.m_weak;
}

/**
 * Creates a new @ref StrongHandle from a @ref WeakHandle.
 */
template <IsAsset A>
[[nodiscard]] auto make_strong(const WeakHandle weak) -> StrongHandle<A> {
    ASSERT(
        weak.id().type() == AssetId::type_id<A>(),
        std::format("WeakHandle type mismatch during promotion! Expected StrongHandle<{}>", typename_of<A>())
    );

    auto& concrete_pool = *static_cast<AssetPool<A>*>(weak.pool());
    return StrongHandle<A>{ weak.id(), concrete_pool, weak.path() };
}

/**
 * Casts a @ref AssetPoolBase to its typed child instance.
 * @tparam A The desired asset type to cast the pool to.
 * @param base The type erased pool instance.
 * @return A typed @ref AssetPool<A> casted instance.
 */
template <IsAsset A>
[[nodiscard]] auto pool_cast(AssetPoolBase* base) -> AssetPool<A>& {
    ASSERT(base != nullptr, "Attempted to pool_cast a nullptr!");
    ASSERT(
        base->type_id() == AssetId::type_id<A>(),
        std::format("Invalid pool cast! Pool holds type {}, expected {}", base->type_id(), AssetId::type_id<A>())
    );
    return *static_cast<AssetPool<A>*>(base);
}

/**
 * Casts a @ref AssetPoolBase to its typed child instance.
 * @tparam A The desired asset type to cast the pool to.
 * @param base The type erased pool instance.
 * @return A typed const @ref AssetPool<A> casted instance.
 */
template <IsAsset A>
[[nodiscard]] auto pool_cast(const AssetPoolBase* base) -> const AssetPool<A>& {
    ASSERT(base != nullptr, "Attempted to pool_cast a nullptr!");
    ASSERT(
        base->type_id() == AssetId::type_id<A>(),
        std::format("Invalid pool cast! Pool holds type {}, expected {}", base->type_id(), AssetId::type_id<A>())
    );
    return *static_cast<const AssetPool<A>*>(base);
}

} // namespace siren
