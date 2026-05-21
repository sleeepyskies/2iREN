#include "shader.hpp"

#include "2iren/rhi/device.hpp"


namespace siren {

Shader::Shader(
    Device* device,
    const ShaderHandle handle
) : Base(device, handle) { }

Shader::~Shader() {
    if (m_device && m_handle.valid()) {
        m_device->destroy_shader(m_handle);
    }
}

Shader::Shader(Shader&& other) noexcept : Base(std::move(other)) { }

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_device && m_handle.valid()) {
            m_device->destroy_shader(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Shader::descriptor() const noexcept -> const ShaderDescriptor& { return m_device->shader_descriptor(m_handle); }
} // namespace siren
