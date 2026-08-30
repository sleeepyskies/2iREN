#include "graphics_pipeline.hpp"

#include "2iREN/graphics/device.hpp"

namespace siren {
GraphicsPipeline::GraphicsPipeline(Device* device, const GraphicsPipelineHandle handle) :
    Base(device, handle) {}

GraphicsPipeline::~GraphicsPipeline() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_graphics_pipeline(m_handle);
    }
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept : Base(std::move(other)) {}

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept {
    if (this != &other) {
        // cleanup old buffer
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_graphics_pipeline(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto GraphicsPipeline::descriptor() const noexcept -> const GraphicsPipelineDescriptor& {
    return m_device->graphics_pipeline_descriptor(m_handle);
}

} // namespace siren
