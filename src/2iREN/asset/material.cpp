#include "material.hpp"

namespace siren {

// setters

auto MaterialAsset::set_base_color(const Rgba& value) noexcept -> void { m_base_color = value; }
auto MaterialAsset::set_metallic(const f32 value) noexcept -> void { m_metallic = value; }
auto MaterialAsset::set_roughness(const f32 value) noexcept -> void { m_roughness = value; }

auto MaterialAsset::set_base_color_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_base_color_tex = handle;
}

auto MaterialAsset::set_metallic_roughness_tex(const StrongHandle<Texture>& handle) noexcept
    -> void {
    m_metallic_roughness_tex = handle;
}

auto MaterialAsset::set_clear_coat(const f32 value) noexcept -> void { m_clear_coat = value; }
auto MaterialAsset::set_clear_coat_roughness(const f32 value) noexcept -> void {
    m_clear_coat_roughness = value;
}

auto MaterialAsset::set_clear_coat_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_clear_coat_tex = handle;
}

auto MaterialAsset::set_clear_coat_roughness_tex(const StrongHandle<Texture>& handle) noexcept
    -> void {
    m_clear_coat_roughness_tex = handle;
}

auto MaterialAsset::set_clearcoat_normal_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_clearcoat_normal_tex = handle;
}

auto MaterialAsset::set_transmission(const f32 value) noexcept -> void { m_transmission = value; }

auto MaterialAsset::set_transmission_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_transmission_tex = handle;
}

auto MaterialAsset::set_thickness(const f32 value) noexcept -> void { m_thickness = value; }

auto MaterialAsset::set_attenuation_color(const Vec3f& value) noexcept -> void {
    m_attenuation_color = value;
}

auto MaterialAsset::set_attenuation_distance(const f32 value) noexcept -> void {
    m_attenuation_distance = value;
}

auto MaterialAsset::set_thickness_texture(const StrongHandle<Texture>& handle) noexcept -> void {
    m_thickness_texture = handle;
}

auto MaterialAsset::set_ior(const f32 value) noexcept -> void { m_ior = value; }
auto MaterialAsset::set_specular_factor(const f32 value) noexcept -> void {
    m_specular_factor = value;
}

auto MaterialAsset::set_specular_color(const Vec3f& value) noexcept -> void {
    m_specular_color = value;
}

auto MaterialAsset::set_specular_color_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_specular_color_tex = handle;
}

auto MaterialAsset::set_specular_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_specular_tex = handle;
}

auto MaterialAsset::set_sheen_color(const Vec3f& value) noexcept -> void { m_sheen_color = value; }

auto MaterialAsset::set_sheen_roughness(const f32 value) noexcept -> void {
    m_sheen_roughness = value;
}

auto MaterialAsset::set_sheen_color_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_sheen_color_tex = handle;
}

auto MaterialAsset::set_sheen_roughness_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_sheen_roughness_tex = handle;
}

auto MaterialAsset::set_emissive_strength(const f32 value) noexcept -> void {
    m_emissive_strength = value;
}

auto MaterialAsset::set_emissive_color(const Vec3f& value) noexcept -> void {
    m_emissive_color = value;
}

auto MaterialAsset::set_emissive_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_emissive_tex = handle;
}

auto MaterialAsset::set_iridescence_factor(const f32 value) noexcept -> void {
    m_iridescence_factor = value;
}
auto MaterialAsset::set_iridescence_ior(const f32 value) noexcept -> void {
    m_iridescence_ior = value;
}
auto MaterialAsset::set_iridescence_min(const f32 value) noexcept -> void {
    m_iridescence_min = value;
}
auto MaterialAsset::set_iridescence_max(const f32 value) noexcept -> void {
    m_iridescence_max = value;
}

auto MaterialAsset::set_iridescence_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_iridescence_tex = handle;
}

auto MaterialAsset::set_iridescence_thickness_tex(const StrongHandle<Texture>& handle) noexcept
    -> void {
    m_iridescence_thickness_tex = handle;
}

auto MaterialAsset::set_diffuse_transmission_factor(f32 value) noexcept -> void {
    m_diffuse_transmission_factor = value;
}

auto MaterialAsset::set_diffuse_transmission_color(const Vec3f& value) noexcept -> void {
    m_diffuse_transmission_color = value;
}

auto MaterialAsset::set_diffuse_transmission_tex(const StrongHandle<Texture>& handle) noexcept
    -> void {
    m_diffuse_transmission_tex = handle;
}

auto MaterialAsset::set_diffuse_transmission_color_tex(const StrongHandle<Texture>& handle) noexcept
    -> void {
    m_diffuse_transmission_color_tex = handle;
}

auto MaterialAsset::set_anisotropy_strength(const f32 value) noexcept -> void {
    m_anisotropy_strength = value;
}

auto MaterialAsset::set_anisotropy_rotation(const f32 value) noexcept -> void {
    m_anisotropy_rotation = value;
}

auto MaterialAsset::set_anisotropy_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_anisotropy_tex = handle;
}

auto MaterialAsset::set_dispersion(const f32 value) noexcept -> void { m_dispersion = value; }

auto MaterialAsset::set_normal_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_normal_tex = handle;
}

auto MaterialAsset::set_occlusion_tex(const StrongHandle<Texture>& handle) noexcept -> void {
    m_occlusion_tex = handle;
}

auto MaterialAsset::set_alpha_mode(const AlphaMode value) noexcept -> void { m_alpha_mode = value; }

auto MaterialAsset::set_alpha_cutoff(const f32 value) noexcept -> void { m_alpha_cutoff = value; }

auto MaterialAsset::set_double_sided(const bool value) noexcept -> void { m_double_sided = value; }

auto MaterialAsset::set_unlit(const bool value) noexcept -> void { m_unlit = value; }

// getters

auto MaterialAsset::base_color() const noexcept -> const Rgba& { return m_base_color; }
auto MaterialAsset::metallic() const noexcept -> f32 { return m_metallic; }
auto MaterialAsset::roughness() const noexcept -> f32 { return m_roughness; }

auto MaterialAsset::base_color_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_base_color_tex;
}

auto MaterialAsset::metallic_roughness_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_metallic_roughness_tex;
}

auto MaterialAsset::clear_coat() const noexcept -> f32 { return m_clear_coat; }
auto MaterialAsset::clear_coat_roughness() const noexcept -> f32 { return m_clear_coat_roughness; }

auto MaterialAsset::clear_coat_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_clear_coat_tex;
}

auto MaterialAsset::clear_coat_roughness_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_clear_coat_roughness_tex;
}

auto MaterialAsset::clearcoat_normal_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_clearcoat_normal_tex;
}

auto MaterialAsset::transmission() const noexcept -> f32 { return m_transmission; }

auto MaterialAsset::transmission_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_transmission_tex;
}

auto MaterialAsset::thickness() const noexcept -> f32 { return m_thickness; }

auto MaterialAsset::attenuation_color() const noexcept -> const Vec3f& {
    return m_attenuation_color;
}

auto MaterialAsset::attenuation_distance() const noexcept -> f32 { return m_attenuation_distance; }

auto MaterialAsset::thickness_texture() const noexcept -> const StrongHandle<Texture>& {
    return m_thickness_texture;
}

auto MaterialAsset::ior() const noexcept -> f32 { return m_ior; }
auto MaterialAsset::specular_factor() const noexcept -> f32 { return m_specular_factor; }

auto MaterialAsset::specular_color() const noexcept -> const Vec3f& { return m_specular_color; }

auto MaterialAsset::specular_color_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_specular_color_tex;
}

auto MaterialAsset::specular_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_specular_tex;
}

auto MaterialAsset::sheen_color() const noexcept -> const Vec3f& { return m_sheen_color; }

auto MaterialAsset::sheen_roughness() const noexcept -> f32 { return m_sheen_roughness; }

auto MaterialAsset::sheen_color_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_sheen_color_tex;
}

auto MaterialAsset::sheen_roughness_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_sheen_roughness_tex;
}

auto MaterialAsset::emissive_strength() const noexcept -> f32 { return m_emissive_strength; }

auto MaterialAsset::emissive_color() const noexcept -> const Vec3f& { return m_emissive_color; }

auto MaterialAsset::emissive_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_emissive_tex;
}

auto MaterialAsset::iridescence_factor() const noexcept -> f32 { return m_iridescence_factor; }
auto MaterialAsset::iridescence_ior() const noexcept -> f32 { return m_iridescence_ior; }
auto MaterialAsset::iridescence_min() const noexcept -> f32 { return m_iridescence_min; }
auto MaterialAsset::iridescence_max() const noexcept -> f32 { return m_iridescence_max; }

auto MaterialAsset::iridescence_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_iridescence_tex;
}

auto MaterialAsset::iridescence_thickness_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_iridescence_thickness_tex;
}

auto MaterialAsset::diffuse_transmission_factor() const noexcept -> f32 {
    return m_diffuse_transmission_factor;
}

auto MaterialAsset::diffuse_transmission_color() const noexcept -> const Vec3f& {
    return m_diffuse_transmission_color;
}

auto MaterialAsset::diffuse_transmission_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_diffuse_transmission_tex;
}

auto MaterialAsset::diffuse_transmission_color_tex() const noexcept
    -> const StrongHandle<Texture>& {
    return m_diffuse_transmission_color_tex;
}

auto MaterialAsset::anisotropy_strength() const noexcept -> f32 { return m_anisotropy_strength; }

auto MaterialAsset::anisotropy_rotation() const noexcept -> f32 { return m_anisotropy_rotation; }

auto MaterialAsset::anisotropy_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_anisotropy_tex;
}

auto MaterialAsset::dispersion() const noexcept -> f32 { return m_dispersion; }

auto MaterialAsset::normal_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_normal_tex;
}

auto MaterialAsset::occlusion_tex() const noexcept -> const StrongHandle<Texture>& {
    return m_occlusion_tex;
}

auto MaterialAsset::alpha_mode() const noexcept -> AlphaMode { return m_alpha_mode; }

auto MaterialAsset::alpha_cutoff() const noexcept -> f32 { return m_alpha_cutoff; }

auto MaterialAsset::double_sided() const noexcept -> bool { return m_double_sided; }

auto MaterialAsset::unlit() const noexcept -> bool { return m_unlit; }

} // namespace siren
