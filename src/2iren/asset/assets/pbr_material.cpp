#include "pbr_material.hpp"


namespace siren {

// setters

void PBRMaterialAsset::set_base_color(const RGBA& value) noexcept { m_base_color = value; }
void PBRMaterialAsset::set_metallic(const f32 value) noexcept { m_metallic = value; }
void PBRMaterialAsset::set_roughness(const f32 value) noexcept { m_roughness = value; }

void PBRMaterialAsset::set_base_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_base_color_tex = handle;
}

void PBRMaterialAsset::set_metallic_roughness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_metallic_roughness_tex = handle;
}

void PBRMaterialAsset::set_clear_coat(const f32 value) noexcept { m_clear_coat = value; }
void PBRMaterialAsset::set_clear_coat_roughness(const f32 value) noexcept { m_clear_coat_roughness = value; }

void PBRMaterialAsset::set_clear_coat_tex(const StrongHandle<Texture>& handle) noexcept {
    m_clear_coat_tex = handle;
}

void PBRMaterialAsset::set_clear_coat_roughness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_clear_coat_roughness_tex = handle;
}

void PBRMaterialAsset::set_clearcoat_normal_tex(const StrongHandle<Texture>& handle) noexcept {
    m_clearcoat_normal_tex = handle;
}

void PBRMaterialAsset::set_transmission(const f32 value) noexcept { m_transmission = value; }

void PBRMaterialAsset::set_transmission_tex(const StrongHandle<Texture>& handle) noexcept {
    m_transmission_tex = handle;
}

void PBRMaterialAsset::set_thickness(const f32 value) noexcept { m_thickness = value; }

void PBRMaterialAsset::set_attenuation_color(const glm::vec3& value) noexcept {
    m_attenuation_color = value;
}

void PBRMaterialAsset::set_attenuation_distance(const f32 value) noexcept {
    m_attenuation_distance = value;
}

void PBRMaterialAsset::set_thickness_texture(const StrongHandle<Texture>& handle) noexcept {
    m_thickness_texture = handle;
}

void PBRMaterialAsset::set_ior(const f32 value) noexcept { m_ior = value; }
void PBRMaterialAsset::set_specular_factor(const f32 value) noexcept { m_specular_factor = value; }

void PBRMaterialAsset::set_specular_color(const glm::vec3& value) noexcept {
    m_specular_color = value;
}

void PBRMaterialAsset::set_specular_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_specular_color_tex = handle;
}

void PBRMaterialAsset::set_specular_tex(const StrongHandle<Texture>& handle) noexcept {
    m_specular_tex = handle;
}

void PBRMaterialAsset::set_sheen_color(const glm::vec3& value) noexcept {
    m_sheen_color = value;
}

void PBRMaterialAsset::set_sheen_roughness(const f32 value) noexcept {
    m_sheen_roughness = value;
}

void PBRMaterialAsset::set_sheen_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_sheen_color_tex = handle;
}

void PBRMaterialAsset::set_sheen_roughness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_sheen_roughness_tex = handle;
}

void PBRMaterialAsset::set_emissive_strength(const f32 value) noexcept {
    m_emissive_strength = value;
}

void PBRMaterialAsset::set_emissive_color(const glm::vec3& value) noexcept {
    m_emissive_color = value;
}

void PBRMaterialAsset::set_emissive_tex(const StrongHandle<Texture>& handle) noexcept {
    m_emissive_tex = handle;
}

void PBRMaterialAsset::set_iridescence_factor(const f32 value) noexcept { m_iridescence_factor = value; }
void PBRMaterialAsset::set_iridescence_ior(const f32 value) noexcept { m_iridescence_ior = value; }
void PBRMaterialAsset::set_iridescence_min(const f32 value) noexcept { m_iridescence_min = value; }
void PBRMaterialAsset::set_iridescence_max(const f32 value) noexcept { m_iridescence_max = value; }

void PBRMaterialAsset::set_iridescence_tex(const StrongHandle<Texture>& handle) noexcept {
    m_iridescence_tex = handle;
}

void PBRMaterialAsset::set_iridescence_thickness_tex(const StrongHandle<Texture>& handle) noexcept {
    m_iridescence_thickness_tex = handle;
}

void PBRMaterialAsset::set_diffuse_transmission_factor(f32 value) noexcept {
    m_diffuse_transmission_factor = value;
}

void PBRMaterialAsset::set_diffuse_transmission_color(const glm::vec3& value) noexcept {
    m_diffuse_transmission_color = value;
}

void PBRMaterialAsset::set_diffuse_transmission_tex(const StrongHandle<Texture>& handle) noexcept {
    m_diffuse_transmission_tex = handle;
}

void PBRMaterialAsset::set_diffuse_transmission_color_tex(const StrongHandle<Texture>& handle) noexcept {
    m_diffuse_transmission_color_tex = handle;
}

void PBRMaterialAsset::set_anisotropy_strength(const f32 value) noexcept {
    m_anisotropy_strength = value;
}

void PBRMaterialAsset::set_anisotropy_rotation(const f32 value) noexcept {
    m_anisotropy_rotation = value;
}

void PBRMaterialAsset::set_anisotropy_tex(const StrongHandle<Texture>& handle) noexcept {
    m_anisotropy_tex = handle;
}

void PBRMaterialAsset::set_dispersion(const f32 value) noexcept {
    m_dispersion = value;
}

void PBRMaterialAsset::set_normal_tex(const StrongHandle<Texture>& handle) noexcept {
    m_normal_tex = handle;
}

void PBRMaterialAsset::set_occlusion_tex(const StrongHandle<Texture>& handle) noexcept {
    m_occlusion_tex = handle;
}

void PBRMaterialAsset::set_alpha_mode(const AlphaMode value) noexcept {
    m_alpha_mode = value;
}

void PBRMaterialAsset::set_alpha_cutoff(const f32 value) noexcept {
    m_alpha_cutoff = value;
}

void PBRMaterialAsset::set_double_sided(const bool value) noexcept {
    m_double_sided = value;
}

void PBRMaterialAsset::set_unlit(const bool value) noexcept {
    m_unlit = value;
}

// getters

const RGBA& PBRMaterialAsset::base_color() const noexcept { return m_base_color; }
f32 PBRMaterialAsset::metallic() const noexcept { return m_metallic; }
f32 PBRMaterialAsset::roughness() const noexcept { return m_roughness; }

const StrongHandle<Texture>& PBRMaterialAsset::base_color_tex() const noexcept {
    return m_base_color_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::metallic_roughness_tex() const noexcept {
    return m_metallic_roughness_tex;
}

f32 PBRMaterialAsset::clear_coat() const noexcept { return m_clear_coat; }
f32 PBRMaterialAsset::clear_coat_roughness() const noexcept { return m_clear_coat_roughness; }

const StrongHandle<Texture>& PBRMaterialAsset::clear_coat_tex() const noexcept {
    return m_clear_coat_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::clear_coat_roughness_tex() const noexcept {
    return m_clear_coat_roughness_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::clearcoat_normal_tex() const noexcept {
    return m_clearcoat_normal_tex;
}

f32 PBRMaterialAsset::transmission() const noexcept { return m_transmission; }

const StrongHandle<Texture>& PBRMaterialAsset::transmission_tex() const noexcept {
    return m_transmission_tex;
}

f32 PBRMaterialAsset::thickness() const noexcept { return m_thickness; }

const glm::vec3& PBRMaterialAsset::attenuation_color() const noexcept {
    return m_attenuation_color;
}

f32 PBRMaterialAsset::attenuation_distance() const noexcept {
    return m_attenuation_distance;
}

const StrongHandle<Texture>& PBRMaterialAsset::thickness_texture() const noexcept {
    return m_thickness_texture;
}

f32 PBRMaterialAsset::ior() const noexcept { return m_ior; }
f32 PBRMaterialAsset::specular_factor() const noexcept { return m_specular_factor; }

const glm::vec3& PBRMaterialAsset::specular_color() const noexcept {
    return m_specular_color;
}

const StrongHandle<Texture>& PBRMaterialAsset::specular_color_tex() const noexcept {
    return m_specular_color_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::specular_tex() const noexcept {
    return m_specular_tex;
}

const glm::vec3& PBRMaterialAsset::sheen_color() const noexcept {
    return m_sheen_color;
}

f32 PBRMaterialAsset::sheen_roughness() const noexcept {
    return m_sheen_roughness;
}

const StrongHandle<Texture>& PBRMaterialAsset::sheen_color_tex() const noexcept {
    return m_sheen_color_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::sheen_roughness_tex() const noexcept {
    return m_sheen_roughness_tex;
}

f32 PBRMaterialAsset::emissive_strength() const noexcept {
    return m_emissive_strength;
}

const glm::vec3& PBRMaterialAsset::emissive_color() const noexcept {
    return m_emissive_color;
}

const StrongHandle<Texture>& PBRMaterialAsset::emissive_tex() const noexcept {
    return m_emissive_tex;
}

f32 PBRMaterialAsset::iridescence_factor() const noexcept { return m_iridescence_factor; }
f32 PBRMaterialAsset::iridescence_ior() const noexcept { return m_iridescence_ior; }
f32 PBRMaterialAsset::iridescence_min() const noexcept { return m_iridescence_min; }
f32 PBRMaterialAsset::iridescence_max() const noexcept { return m_iridescence_max; }

const StrongHandle<Texture>& PBRMaterialAsset::iridescence_tex() const noexcept {
    return m_iridescence_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::iridescence_thickness_tex() const noexcept {
    return m_iridescence_thickness_tex;
}

f32 PBRMaterialAsset::diffuse_transmission_factor() const noexcept {
    return m_diffuse_transmission_factor;
}

const glm::vec3& PBRMaterialAsset::diffuse_transmission_color() const noexcept {
    return m_diffuse_transmission_color;
}

const StrongHandle<Texture>& PBRMaterialAsset::diffuse_transmission_tex() const noexcept {
    return m_diffuse_transmission_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::diffuse_transmission_color_tex() const noexcept {
    return m_diffuse_transmission_color_tex;
}

f32 PBRMaterialAsset::anisotropy_strength() const noexcept {
    return m_anisotropy_strength;
}

f32 PBRMaterialAsset::anisotropy_rotation() const noexcept {
    return m_anisotropy_rotation;
}

const StrongHandle<Texture>& PBRMaterialAsset::anisotropy_tex() const noexcept {
    return m_anisotropy_tex;
}

f32 PBRMaterialAsset::dispersion() const noexcept {
    return m_dispersion;
}

const StrongHandle<Texture>& PBRMaterialAsset::normal_tex() const noexcept {
    return m_normal_tex;
}

const StrongHandle<Texture>& PBRMaterialAsset::occlusion_tex() const noexcept {
    return m_occlusion_tex;
}

AlphaMode PBRMaterialAsset::alpha_mode() const noexcept {
    return m_alpha_mode;
}

f32 PBRMaterialAsset::alpha_cutoff() const noexcept {
    return m_alpha_cutoff;
}

bool PBRMaterialAsset::double_sided() const noexcept {
    return m_double_sided;
}

bool PBRMaterialAsset::unlit() const noexcept {
    return m_unlit;
}

} // namespace siren
