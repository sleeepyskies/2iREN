#pragma once

#include <utility>
#include <vector>
#include <libassert/assert.hpp>

#include "base.hpp"
#include "sync/rw_lock.hpp"


namespace siren {

class Device;


namespace detail {
struct NullHandle_t {
    template <typename Handle>
    constexpr operator Handle() const { return Handle::invalid(); }
};
}


inline constexpr auto NullHandle = detail::NullHandle_t{ };

/**
 * @struct ResourceHandle
 * @brief A packed 32-bit generational handle.
 * * Bit layout:
 * - [16-31] Generation: Incremented on slot reuse to prevent stale references.
 * - [00-15] Index: The lookup key into the resource pool.
 */
template <typename Tag>
struct ResourceHandle {
    using IdType         = u32;
    using IndexType      = u16;
    using GenerationType = u16;

    static constexpr IdType IndexMask = bit(16) - 1; // 0xffff

    ResourceHandle() : packed_id(0) { }
    ResourceHandle(const IndexType idx, const GenerationType gen)
        : packed_id(static_cast<IdType>(gen << 16) | (idx & IndexMask)) { }

    ResourceHandle(const ResourceHandle&)            = default;
    ResourceHandle& operator=(const ResourceHandle&) = default;
    ResourceHandle(ResourceHandle&&)                 = default;
    ResourceHandle& operator=(ResourceHandle&&)      = default;

    /** @brief Simple factory method to return an invalid Identifier64. */
    [[nodiscard]]
    constexpr static auto invalid() noexcept -> ResourceHandle { return { }; }

    /** @brief Returns the full packed value of this id. */
    [[nodiscard]] constexpr auto packed() const noexcept -> IdType { return packed_id; }
    /** @brief Returns the index of this id. */
    [[nodiscard]] constexpr auto index() const noexcept -> IndexType {
        return static_cast<IndexType>(packed_id & IndexMask);
    }
    /** @brief Returns the generation of this id. */
    [[nodiscard]] constexpr auto generation() const noexcept -> GenerationType {
        return static_cast<GenerationType>(packed_id) >> 16;
    }

    /** @brief Checks if the handle is valid aka has a non 0 inner value. */
    [[nodiscard]] constexpr auto is_valid() const noexcept -> bool { return packed_id != 0; }
    /** @brief Kills the handle by zeroing its value. */
    constexpr auto invalidate() noexcept -> void { packed_id = 0; }
    /** @copydoc is_valid */
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

    /** @brief Default comparison operator. */
    friend bool operator==(const ResourceHandle&, const ResourceHandle&) = default;

    IdType packed_id;
};

/**
 * @brief Simple base struct to identify render resources.
 * Enforces disabling copies on all RenderResource's, as well
 * as provides access to the Device.
 * @note All RenderResources' are immutable. In order to update
 * some objects properties, it must be instead destroyed and
 * replaced by a new one.
 * @note Any subclasses should define move constructors, and
 * make sure to call the move constructor of this class as well.
 * See examples is @ref Buffer.
 */
template <typename Resource>
class RenderResource {
public:
    friend class Device;

    using HandleType = ResourceHandle<Resource>;

    RenderResource() : m_device(nullptr) { }
    RenderResource(Device* device, HandleType handle) : m_device(device), m_handle(handle) { }
    ~RenderResource() = default;

    RenderResource(const RenderResource&)            = delete;
    RenderResource& operator=(const RenderResource&) = delete;
    RenderResource(RenderResource&& other) noexcept
        : m_device(std::exchange(other.m_device, nullptr)), m_handle(std::exchange(other.m_handle, { })) { }
    RenderResource& operator=(RenderResource&& other) noexcept {
        if (this != &other) {
            m_device = std::exchange(other.m_device, nullptr);
            m_handle = std::exchange(other.m_handle, { });
        }
        return *this;
    }

    /** @brief Returns the underlying native handle for this resource. */
    [[nodiscard]] auto handle() const noexcept -> HandleType { return m_handle; }

protected:
    Device* m_device;
    HandleType m_handle;
};

struct Nothing { };

/**
 * @class RenderResourceTable
 * @brief Manages mapping RenderResourceID's (siren proxy handles) to the
 * graphics API's handles.
 * @tparam ApiHandle The API's handle type aka GLuint or VkBuffer etc.
 * @tparam Resource The 2iren resource being managed.
 * @tparam Extra Some additional data to store with each resource.
 * @note We store some resource related items in the table since they
 * are API specific, and thus the 2iren objects would need to be specialized.
 */
template <typename ApiHandle, typename Resource, typename Extra = Nothing>
class RenderResourceTable {
public:
    using ApiHandleType = ApiHandle;
    using HandleType    = ResourceHandle<Resource>;

private:
    using IndexType      = HandleType::IndexType;
    using GenerationType = HandleType::GenerationType;

    /** @brief Struct used for storing resource data. */
    struct TableEntry {
        /** @brief The actual api handle. */
        ApiHandleType api_handle = 0;
        /** @brief The generation of this resource's slot. */
        GenerationType generation = 0;
        /** @brief Some extra data that the user may define. */
        Extra extra = { };

        // @formatter:off
        void kill() { ++generation; api_handle = 0; extra = Extra{ }; }
        // @formatter:on
    };

    /** @brief Container for inner data of the RenderResourceTable. */
    struct Inner {
        /** @brief The stored API handles with generation counting. */
        std::vector<TableEntry> table;
        /** @brief Any free indices to use. */
        std::vector<IndexType> free_list;
    };

public:
    /** @brief Creates and returns a new proxy handle with no api handle associated with it. */
    [[nodiscard]] auto reserve() -> HandleType {
        IndexType index;

        auto inner = m_inner.write();

        if (!inner->free_list.empty()) {
            // there's a free index,
            index = inner->free_list.back();
            inner->free_list.pop_back();
        } else {
            // fetch a new index
            index = static_cast<IndexType>(inner->table.size());
            inner->table.emplace_back(); // grow table to avoid arr index errors
        }

        TableEntry& table_entry = inner->table[index];

        return HandleType{ index, table_entry.generation };
    }

    /** @brief Takes a proxy handle (should be generated by this table), and associates it with the api handle. */
    auto link(
        const HandleType proxy_handle,
        const ApiHandleType api_handle,
        const Extra& extra
    ) -> void {
        auto inner = m_inner.write();
        ASSERT(is_valid_id(proxy_handle, *inner), "Passed an invalid ProxyHandleType: {}", proxy_handle);
        auto& table_entry      = inner->table[proxy_handle.index()];
        table_entry.api_handle = api_handle;
        table_entry.extra      = extra;
    }

    /** @brief Frees the proxy handle. */
    auto release(const HandleType proxy_handle) -> void {
        auto inner = m_inner.write();
        ASSERT(is_valid_id(proxy_handle, *inner), "Cannot free an invalid ProxyHandleType: {}", proxy_handle);
        TableEntry& table_entry = inner->table[proxy_handle.index()];
        inner->free_list.emplace_back(proxy_handle.index());
        table_entry.kill();
    }

    /** @brief Gets the api handle associated with this proxy handle iff valid. */
    [[nodiscard]] auto fetch(const HandleType proxy_handle) const noexcept -> ApiHandleType {
        auto inner = m_inner.read();
        if (!is_valid_id(proxy_handle, *inner)) { return ApiHandleType{ 0 }; }
        return inner->table[proxy_handle.index()].api_handle;
    }

    /** @brief Gets the extra data associated with this proxy. */
    [[nodiscard]] auto extra(const HandleType proxy_handle) noexcept -> std::optional<std::reference_wrapper<Extra>> {
        auto inner = m_inner.read();
        if (!is_valid_id(proxy_handle, *inner)) { return std::nullopt; }
        return std::ref(inner->table[proxy_handle.index()].extra);
    }

    /** @brief Gets the extra data associated with this proxy. */
    [[nodiscard]] auto extra(
        const HandleType proxy_handle
    ) const noexcept -> std::optional<std::reference_wrapper<const Extra>> {
        auto inner = m_inner.read();
        if (!is_valid_id(proxy_handle, *inner)) { return std::nullopt; }
        return std::cref(inner->table[proxy_handle.index()].extra);
    }

private:
    /**
     * @brief Checks if a given handle is valid.
     * @todo Is very strict, maybe we want to alter these check conditions,
     * or at least no assert this condition.
     */
    auto is_valid_id(const HandleType proxy_handle, const Inner& inner) const -> bool {
        if (proxy_handle.index() >= inner.table.size() || !proxy_handle.is_valid()) { return false; }
        const auto& entry = inner.table[proxy_handle.index()];
        return entry.generation == proxy_handle.generation();
    }

    RwLock<Inner> m_inner;
};

} // namespace siren

template <typename Tag>
struct std::hash<siren::ResourceHandle<Tag>> {
    auto operator()(const siren::ResourceHandle<Tag>& identifier) const noexcept -> siren::usize {
        return static_cast<siren::usize>(identifier.packed());
    }
};
