#pragma once

#include <utility>
#include <vector>

#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/utility/identifier.hpp"

namespace siren {

class Device;

/// @struct ResourceHandle
/// @todo inherit from @ref Identifier here instead of copy paste.
/// @brief A packed 32-bit generational handle.
/// * Bit layout:
/// - [16-31] Generation: Incremented on slot reuse to prevent stale references.
/// - [00-15] Index: The lookup key into the resource pool.
template <typename Tag>
struct ResourceHandle : Identifier<ResourceHandle<Tag>> {
    using Base = Identifier<ResourceHandle<Tag>>;
    using Base::Base;

    using IdType         = Base::IdType;
    using IndexType      = Base::IndexType;
    using GenerationType = Base::GenerationType;

    /// @brief Constructs a new ResourceHandle.
    /// @param idx The index of the resource in storage.
    /// @param gen The generation of the ResourceHandle for this storage slot.
    ResourceHandle(const IndexType idx, const GenerationType gen) noexcept : Base(idx, gen, 0) { }

    static constexpr auto invalid() noexcept -> ResourceHandle {
        return ResourceHandle{};
    }

    /// @brief Stringifies the ResourceHandle.
    [[nodiscard]] constexpr auto to_string() const noexcept -> std::string {
        return std::format(
            "{}Handle(index={}, generation={})",
            typename_of<Tag>(),
            this->index(),
            this->generation()
        );
    }
};

/// @brief Simple base struct to identify render resources.
/// Enforces disabling copies on all RenderResource's, as well
/// as provides access to the Device.
/// @note Any subclasses should define move constructors, and
/// make sure to call the move constructor of this class as well.
/// See examples is @ref Buffer.
template <typename Resource>
class RenderResource {
public:
    friend class Device;

    using HandleType = ResourceHandle<Resource>;

    RenderResource() : m_device(nullptr), m_handle(NullHandle) { }
    RenderResource(Device* device, HandleType handle) : m_device(device), m_handle(handle) { }
    ~RenderResource() = default;

    RenderResource(const RenderResource&)            = delete;
    RenderResource& operator=(const RenderResource&) = delete;
    RenderResource(RenderResource&& other) noexcept :
        m_device(std::exchange(other.m_device, nullptr)),
        m_handle(std::exchange(other.m_handle, {})) { }
    RenderResource& operator=(RenderResource&& other) noexcept {
        if (this != &other) {
            m_device = std::exchange(other.m_device, nullptr);
            m_handle = std::exchange(other.m_handle, {});
        }
        return *this;
    }

    /// @brief Returns the underlying native handle for this resource.
    [[nodiscard]]
    auto handle() const noexcept -> HandleType {
        return m_handle;
    }

    /// @brief Returns the string representation of this ResourceHandle.
    [[nodiscard]]
    auto to_string() const noexcept -> std::string {
        return std::format("{}({})", typename_of<Resource>(), m_handle.packed());
    }

protected:
    Device* m_device;
    HandleType m_handle;
};

namespace detail {
struct Nothing { };
} // namespace detail

/// @class RenderResourceTable
/// @brief Manages mapping RenderResourceID's (siren proxy handles) to the
/// graphics API's handles.
/// @tparam ApiHandle The API's handle type aka GLuint or VkBuffer etc.
/// @tparam Resource The 2iREN resource being managed.
/// @tparam Details Some additional data to store with each resource.
/// @note We store some resource related items in the table since they
/// are API specific, and thus the 2iREN objects would need to be specialized.
template <typename ApiHandle, typename Resource, typename Details = detail::Nothing>
class RenderResourceTable {
public:
    using ApiHandleType = ApiHandle;
    using HandleType    = ResourceHandle<Resource>;

private:
    using IndexType      = HandleType::IndexType;
    using GenerationType = HandleType::GenerationType;

    /// @brief Struct used for storing resource data.
    struct TableEntry {
        /// @brief The actual api handle.
        ApiHandleType api_handle  = 0;
        /// @brief The generation of this resource's slot.
        GenerationType generation = 0;
        /// @brief Some extra data that the user may define.
        Details details           = {};

        auto kill() -> void {
            ++generation;
            api_handle = 0;
            details    = Details{};
        }
    };

public:
    /// @brief Creates and returns a new proxy handle with no api handle
    /// associated with it.
    [[nodiscard]] auto reserve() -> HandleType {
        IndexType index;

        if (!m_free_list.empty()) {
            // there's a free index,
            index = m_free_list.back();
            m_free_list.pop_back();
        } else {
            // fetch a new index
            index = static_cast<IndexType>(m_table.size());
            m_table.emplace_back(); // grow table to avoid arr index errors
        }

        TableEntry& table_entry = m_table[index];

        return HandleType{index, table_entry.generation};
    }

    /// @brief Takes a proxy handle (should be generated by this table), and
    /// associates it with the api handle.
    auto link(const HandleType proxy_handle, const ApiHandleType api_handle, Details&& details)
        -> void {
        ASSERT(is_valid_id(proxy_handle), "passed an invalid ProxyHandleType: {}", proxy_handle);
        auto& table_entry      = m_table[proxy_handle.index()];
        table_entry.api_handle = api_handle;
        table_entry.details    = std::move(details);
    }

    [[nodiscard]]
    auto reserve_link(const ApiHandleType api_handle, Details&& details) -> HandleType {
        const auto handle = reserve();
        link(handle, api_handle, std::move(details));
        return handle;
    }

    /// @brief Frees the proxy handle.
    auto release(const HandleType proxy_handle) -> void {
        ASSERT(
            is_valid_id(proxy_handle), "cannot free an invalid ProxyHandleType: {}", proxy_handle
        );
        TableEntry& table_entry = m_table[proxy_handle.index()];
        m_free_list.emplace_back(proxy_handle.index());
        table_entry.kill();
    }

    /// @brief Frees and returns the proxy handle.
    auto fetch_release(const HandleType proxy) -> ApiHandleType {
        auto handle = fetch(proxy);
        release(proxy);
        return handle;
    }

    /// @brief Gets the api handle associated with this proxy handle iff valid.
    [[nodiscard]] auto fetch(const HandleType proxy_handle) const noexcept -> ApiHandleType {
        ASSERT(is_valid_id(proxy_handle), "Attempting to fetch an invalid handle.");
        return m_table[proxy_handle.index()].api_handle;
    }

    /// @brief Gets the extra data associated with this proxy.
    template <typename Self>
    [[nodiscard]]
    auto details(this Self&& self, const HandleType handle) noexcept -> auto&& {
        ASSERT(
            std::forward<Self>(self).is_valid_id(handle),
            std::format("invalid handle: {}", handle.packed())
        );
        return std::forward<Self>(self).m_table[handle.index()].details;
    }

    /// @brief Returns the number of active resources being managed.
    [[nodiscard]]
    auto count() const noexcept -> u32 {
        return m_table.size();
    }

private:
    /// @brief Checks if a given handle is valid.
    /// @todo Is very strict, maybe we want to alter these check conditions,
    /// or at least no assert this condition.
    auto is_valid_id(const HandleType proxy_handle) const -> bool {
        if (proxy_handle.index() >= m_table.size() || !proxy_handle.is_valid()) {
            return false;
        }
        const auto& entry = m_table[proxy_handle.index()];
        return entry.generation == proxy_handle.generation();
    }

    /// @brief The stored API handles with generation counting.
    std::vector<TableEntry> m_table;
    /// @brief Any free indices to use.
    std::vector<IndexType> m_free_list;
};

} // namespace siren

template <typename Tag>
struct std::hash<siren::ResourceHandle<Tag>> {
    auto operator()(const siren::ResourceHandle<Tag>& identifier) const noexcept -> siren::usize {
        return static_cast<siren::usize>(identifier.packed());
    }
};
