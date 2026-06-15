#include "sampler.hpp"

#include "2iren/rhi/device.hpp"


namespace siren {

Sampler::Sampler(
    Device* device,
    const SamplerHandle handle
) : Base(device, handle) { }

Sampler::~Sampler() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_sampler(m_handle);
    }
}

Sampler::Sampler(Sampler&& other) noexcept
    : Base(std::move(other)) { }

Sampler& Sampler::operator=(Sampler&& other) noexcept {
    if (this != &other) {
        // cleanup old sampler
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_sampler(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Sampler::descriptor() const noexcept -> const SamplerDescriptor& { return m_device->sampler_descriptor(m_handle); }

} // namespace siren
