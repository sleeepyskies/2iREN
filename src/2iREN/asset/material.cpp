#include "material.hpp"

namespace siren {

// setters

void MaterialAsset::set_base_color(const Rgba& value) noexcept { m_base_color = value; }
void MaterialAsset::set_metallic(const f32 value) noexcept { m_metallic = value; }
void MaterialAsset::set_roughness(const f32 value) noexcept { m_roughness = value; }

void MaterialAsset::set_base_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_base_color_tex = handle;
}

void MaterialAsset::set_metallic_roughness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_metallic_roughness_tex = handle;
}

void MaterialAsset::set_clear_coat(const f32 value) noexcept { m_clear_coat = value; }
void MaterialAsset::set_clear_coat_roughness(const f32 value) noexcept {
    m_clear_coat_roughness = value;
}

void MaterialAsset::set_clear_coat_tex(const StrongHandle<Texture>& handle) noexcept {
    m_clear_coat_tex = handle;
}

void MaterialAsset::set_clear_coat_roughness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_clear_coat_roughness_tex = handle;
}

void MaterialAsset::set_clearcoat_normal_tex(const StrongHandle<Texture>& handle) noexcept {
    m_clearcoat_normal_tex = handle;
}

void MaterialAsset::set_transmission(const f32 value) noexcept { m_transmission = value; }

void MaterialAsset::set_transmission_tex(const StrongHandle<Texture>& handle) noexcept {
    m_transmission_tex = handle;
}

void MaterialAsset::set_thickness(const f32 value) noexcept { m_thickness = value; }

void MaterialAsset::set_attenuation_color(const glm::vec3& value) noexcept {
    m_attenuation_color = value;
}

void MaterialAsset::set_attenuation_distance(const f32 value) noexcept {
    m_attenuation_distance = value;
}

void MaterialAsset::set_thickness_texture(const StrongHandle<Texture>& handle) noexcept {
    m_thickness_texture = handle;
}

void MaterialAsset::set_ior(const f32 value) noexcept { m_ior = value; }
void MaterialAsset::set_specular_factor(const f32 value) noexcept { m_specular_factor = value; }

void MaterialAsset::set_specular_color(const glm::vec3& value) noexcept {
    m_specular_color = value;
}

void MaterialAsset::set_specular_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_specular_color_tex = handle;
}

void MaterialAsset::set_specular_tex(const StrongHandle<Texture>& handle) noexcept {
    m_specular_tex = handle;
}

void MaterialAsset::set_sheen_color(const glm::vec3& value) noexcept { m_sheen_color = value; }

void MaterialAsset::set_sheen_roughness(const f32 value) noexcept { m_sheen_roughness = value; }

void MaterialAsset::set_sheen_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_sheen_color_tex = handle;
}

void MaterialAsset::set_sheen_roughness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_sheen_roughness_tex = handle;
}

void MaterialAsset::set_emissive_strength(const f32 value) noexcept { m_emissive_strength = value; }

void MaterialAsset::set_emissive_color(const glm::vec3& value) noexcept {
    m_emissive_color = value;
}

void MaterialAsset::set_emissive_tex(const StrongHandle<Texture>& handle) noexcept {
    m_emissive_tex = handle;
}

void MaterialAsset::set_iridescence_factor(const f32 value) noexcept {
    m_iridescence_factor = value;
}
void MaterialAsset::set_iridescence_ior(const f32 value) noexcept { m_iridescence_ior = value; }
void MaterialAsset::set_iridescence_min(const f32 value) noexcept { m_iridescence_min = value; }
void MaterialAsset::set_iridescence_max(const f32 value) noexcept { m_iridescence_max = value; }

void MaterialAsset::set_iridescence_tex(const StrongHandle<Texture>& handle) noexcept {
    m_iridescence_tex = handle;
}

void MaterialAsset::set_iridescence_thickness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_iridescence_thickness_tex = handle;
}

void MaterialAsset::set_diffuse_transmission_factor(f32 value) noexcept {
    m_diffuse_transmission_factor = value;
}

void MaterialAsset::set_diffuse_transmission_color(const glm::vec3& value) noexcept {
    m_diffuse_transmission_color = value;
}

void MaterialAsset::set_diffuse_transmission_tex(const StrongHandle<Texture>& handle) noexcept {
    m_diffuse_transmission_tex = handle;
}

void MaterialAsset::set_diffuse_transmission_color_tex(
    const StrongHandle<Texture>& handle
) noexcept {
    m_diffuse_transmission_color_tex = handle;
}

void MaterialAsset::set_anisotropy_strength(const f32 value) noexcept {
    m_anisotropy_strength = value;
}

void MaterialAsset::set_anisotropy_rotation(const f32 value) noexcept {
    m_anisotropy_rotation = value;
}

void MaterialAsset::set_anisotropy_tex(const StrongHandle<Texture>& handle) noexcept {
    m_anisotropy_tex = handle;
}

void MaterialAsset::set_dispersion(const f32 value) noexcept { m_dispersion = value; }

void MaterialAsset::set_normal_tex(const StrongHandle<Texture>& handle) noexcept {
    m_normal_tex = handle;
}

void MaterialAsset::set_occlusion_tex(const StrongHandle<Texture>& handle) noexcept {
    m_occlusion_tex = handle;
}

void MaterialAsset::set_alpha_mode(const AlphaMode value) noexcept { m_alpha_mode = value; }

void MaterialAsset::set_alpha_cutoff(const f32 value) noexcept { m_alpha_cutoff = value; }

void MaterialAsset::set_double_sided(const bool value) noexcept { m_double_sided = value; }

void MaterialAsset::set_unlit(const bool value) noexcept { m_unlit = value; }

// getters

const Rgba& MaterialAsset::base_color() const noexcept { return m_base_color; }
f32 MaterialAsset::metallic() const noexcept { return m_metallic; }
f32 MaterialAsset::roughness() const noexcept { return m_roughness; }

const StrongHandle<Texture>& MaterialAsset::base_color_tex() const noexcept {
    return m_base_color_tex;
}

const StrongHandle<Texture>& MaterialAsset::metallic_roughness_tex() const noexcept {
    return m_metallic_roughness_tex;
}

f32 MaterialAsset::clear_coat() const noexcept { return m_clear_coat; }
f32 MaterialAsset::clear_coat_roughness() const noexcept { return m_clear_coat_roughness; }

const StrongHandle<Texture>& MaterialAsset::clear_coat_tex() const noexcept {
    return m_clear_coat_tex;
}

const StrongHandle<Texture>& MaterialAsset::clear_coat_roughness_tex() const noexcept {
    return m_clear_coat_roughness_tex;
}

const StrongHandle<Texture>& MaterialAsset::clearcoat_normal_tex() const noexcept {
    return m_clearcoat_normal_tex;
}

f32 MaterialAsset::transmission() const noexcept { return m_transmission; }

const StrongHandle<Texture>& MaterialAsset::transmission_tex() const noexcept {
    return m_transmission_tex;
}

f32 MaterialAsset::thickness() const noexcept { return m_thickness; }

const glm::vec3& MaterialAsset::attenuation_color() const noexcept { return m_attenuation_color; }

f32 MaterialAsset::attenuation_distance() const noexcept { return m_attenuation_distance; }

const StrongHandle<Texture>& MaterialAsset::thickness_texture() const noexcept {
    return m_thickness_texture;
}

f32 MaterialAsset::ior() const noexcept { return m_ior; }
f32 MaterialAsset::specular_factor() const noexcept { return m_specular_factor; }

const glm::vec3& MaterialAsset::specular_color() const noexcept { return m_specular_color; }

const StrongHandle<Texture>& MaterialAsset::specular_color_tex() const noexcept {
    return m_specular_color_tex;
}

const StrongHandle<Texture>& MaterialAsset::specular_tex() const noexcept { return m_specular_tex; }

const glm::vec3& MaterialAsset::sheen_color() const noexcept { return m_sheen_color; }

f32 MaterialAsset::sheen_roughness() const noexcept { return m_sheen_roughness; }

const StrongHandle<Texture>& MaterialAsset::sheen_color_tex() const noexcept {
    return m_sheen_color_tex;
}

const StrongHandle<Texture>& MaterialAsset::sheen_roughness_tex() const noexcept {
    return m_sheen_roughness_tex;
}

f32 MaterialAsset::emissive_strength() const noexcept { return m_emissive_strength; }

const glm::vec3& MaterialAsset::emissive_color() const noexcept { return m_emissive_color; }

const StrongHandle<Texture>& MaterialAsset::emissive_tex() const noexcept { return m_emissive_tex; }

f32 MaterialAsset::iridescence_factor() const noexcept { return m_iridescence_factor; }
f32 MaterialAsset::iridescence_ior() const noexcept { return m_iridescence_ior; }
f32 MaterialAsset::iridescence_min() const noexcept { return m_iridescence_min; }
f32 MaterialAsset::iridescence_max() const noexcept { return m_iridescence_max; }

const StrongHandle<Texture>& MaterialAsset::iridescence_tex() const noexcept {
    return m_iridescence_tex;
}

const StrongHandle<Texture>& MaterialAsset::iridescence_thickness_tex() const noexcept {
    return m_iridescence_thickness_tex;
}

f32 MaterialAsset::diffuse_transmission_factor() const noexcept {
    return m_diffuse_transmission_factor;
}

const glm::vec3& MaterialAsset::diffuse_transmission_color() const noexcept {
    return m_diffuse_transmission_color;
}

const StrongHandle<Texture>& MaterialAsset::diffuse_transmission_tex() const noexcept {
    return m_diffuse_transmission_tex;
}

const StrongHandle<Texture>& MaterialAsset::diffuse_transmission_color_tex() const noexcept {
    return m_diffuse_transmission_color_tex;
}

f32 MaterialAsset::anisotropy_strength() const noexcept { return m_anisotropy_strength; }

f32 MaterialAsset::anisotropy_rotation() const noexcept { return m_anisotropy_rotation; }

const StrongHandle<Texture>& MaterialAsset::anisotropy_tex() const noexcept {
    return m_anisotropy_tex;
}

f32 MaterialAsset::dispersion() const noexcept { return m_dispersion; }

const StrongHandle<Texture>& MaterialAsset::normal_tex() const noexcept { return m_normal_tex; }

const StrongHandle<Texture>& MaterialAsset::occlusion_tex() const noexcept {
    return m_occlusion_tex;
}

AlphaMode MaterialAsset::alpha_mode() const noexcept { return m_alpha_mode; }

f32 MaterialAsset::alpha_cutoff() const noexcept { return m_alpha_cutoff; }

bool MaterialAsset::double_sided() const noexcept { return m_double_sided; }

bool MaterialAsset::unlit() const noexcept { return m_unlit; }

} // namespace siren
