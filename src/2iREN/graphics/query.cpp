#include "query.hpp"

#include "2iREN/graphics/device.hpp"

namespace siren {
Query::Query(Device* device, const QueryHandle handle) : Base(device, handle) {}

Query::~Query() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_query(m_handle);
    }
}

Query::Query(Query&& other) noexcept : Base(std::move(other)) {}

Query& Query::operator=(Query&& other) noexcept {
    if (this != &other) {
        // cleanup old sampler
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_query(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Query::descriptor() const -> const QueryDescriptor& {
    return m_device->query_descriptor(m_handle);
}
} // namespace siren

