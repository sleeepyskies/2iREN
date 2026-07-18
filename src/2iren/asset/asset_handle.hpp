#pragma once

#include <libassert/assert.hpp>

#include "2iren/util/log.hpp"
#include "asset_id.hpp"
#include "asset_path.hpp"
#include "asset_pool.hpp"

namespace siren {

/**
 * @brief A weak, type erased asset handle.
 * @todo A promote function would be nice, but I cant be bothered to deal with shitty cpp circular
 * headaches rn.
 */
class WeakHandle {
public:
    using TypeID = AssetId::TypeID;

    /** @brief Default constructs an invalid handle. */
    WeakHandle() = default;
    /** @brief Constructs a new WeakHandle. */
    WeakHandle(const AssetId id, AssetPoolBase* pool, const AssetPath& path) : m_path(path), m_id(id), m_pool(pool) {}

    /** @brief Constructs a new invalid WeakHandle. */
    static auto invalid() -> WeakHandle { return WeakHandle{}; }

    WeakHandle(const WeakHandle&)            = default;
    WeakHandle& operator=(const WeakHandle&) = default;
    WeakHandle(WeakHandle&&)                 = default;
    WeakHandle& operator=(WeakHandle&&)      = default;

    /** @brief Returns the @ref AssetID of this WeakHandle. */
    [[nodiscard]] auto id() const noexcept -> AssetId { return m_id; }
    /** @brief Returns the @ref AssetPoolBase pointer of this WeakHandle. */
    [[nodiscard]]
    constexpr auto pool() const noexcept -> AssetPoolBase* {
        return m_pool;
    }
    /** @brief Returns the original asset path of this WeakHandle. */
    [[nodiscard]] constexpr auto path() const noexcept -> const AssetPath& { return m_path; }
    /** @brief Returns the string representation of this handle. */
    [[nodiscard]] auto to_string() const -> std::string { return std::format("Weak({})", m_id); }

    /** @brief Equality comparison operator. */
    [[nodiscard]] constexpr auto operator==(const WeakHandle& other) const -> bool { return id() == other.id(); }

private:
    /** @brief The @ref AssetPath to the referenced asset. */
    AssetPath m_path = AssetPath::invalid();
    /** @brief The raw untyped handle. */
    AssetId m_id = NullHandle;
    /** @brief The pool this handles asset belongs to. */
    AssetPoolBase* m_pool{ nullptr };
};

/**
 * @brief A typed strong handle for assets.
 *
 * @details
 *  As long as at least one @c StrongHandle exists for an asset,
 *  it will remain loaded. As soon as the number of handles reaches
 *  zero, a request will be made to purge the asset at the end of
 *  the frame. If then there are still no references, the asset
 *  will be removed.
 *
 * @tparam A The type of the asset this handle references.
 *
 */
template <typename A>
class StrongHandle {
    // enforce here not in template declaration to avoid issues with recursive types, like GltfNode.
    static_assert(siren::IsAsset<A>, "StrongHandle can only be used with types derived from Asset.");

public:
    using TypeID = WeakHandle::TypeID;

    /** @brief Returns a dummy AssetHandle. */
    static auto invalid() noexcept -> StrongHandle { return StrongHandle{}; }
    ~StrongHandle() {
        if (m_weak.pool()) {
            pool().dec_ref(id());
        }
    }

    StrongHandle(const AssetId& id, AssetPool<A>& pool, const AssetPath& asset_path) :
        m_weak(WeakHandle{ id, &pool, asset_path }) {
        ASSERT(AssetId::type_id<A>() == id.type(),
                "Cannot construct a StrongHandle if AssetID and AssetPool types do not match.");
        pool.inc_ref(id);
    }

    StrongHandle(const StrongHandle& other) : m_weak(other.m_weak) {
        if (m_weak.pool()) {
            pool().inc_ref(id());
        }
    }
    StrongHandle& operator=(const StrongHandle& other) {
        if (this != &other) {
            m_weak = other.m_weak;
            if (m_weak.pool()) {
                pool().inc_ref(id());
            }
        }
        return *this;
    }

    StrongHandle(StrongHandle&& other) noexcept : m_weak(std::move(other.m_weak)) {
        other.m_weak = WeakHandle::invalid();
    }
    StrongHandle& operator=(StrongHandle&& other) noexcept {
        if (this != &other) {
            if (m_weak.pool()) {
                pool().dec_ref(id());
            }
            m_weak       = other.m_weak;
            other.m_weak = WeakHandle::invalid();
        }
        return *this;
    }

    /** @brief Checks if this handle is valid and references an alive asset. */
    [[nodiscard]] auto is_valid() const -> bool {
        return id().is_valid() && m_weak.pool() != nullptr && pool().is_valid_id(id());
    }

    /** @brief Returns the raw untyped version of this handle. */
    [[nodiscard]] constexpr auto id() const noexcept -> AssetId { return m_weak.id(); }
    /** @brief Returns the typed AssetPool reference where the referenced asset is stored. */
    [[nodiscard]] auto pool() const -> AssetPool<A>& { return *dynamic_cast<AssetPool<A>*>(m_weak.pool()); }
    /** @brief Returns the AssetPath of the referenced asset. */
    [[nodiscard]] auto path() const -> AssetPath { return m_weak.path(); }
    /** @brief Returns the string representation of this handle. */
    [[nodiscard]] auto to_string() const -> std::string {
        return std::format("Strong<{}>({})", typename_of<A>(), m_weak.id().packed());
    }

    /** @brief Equality comparison operator. */
    [[nodiscard]] constexpr auto operator==(const StrongHandle& other) const -> bool = default;

private:
    StrongHandle() = default;
    template <IsAsset T>
    friend WeakHandle make_weak(const StrongHandle<T>&) noexcept;

    /** @brief The wrapped @ref WeakHandle. */
    WeakHandle m_weak;
};

} // namespace siren

template <>
struct std::hash<siren::WeakHandle> {
    auto operator()(const siren::WeakHandle& handle) const noexcept -> siren::usize { return handle.id().hash(); }
};
