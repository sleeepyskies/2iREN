#pragma once

#include <string>

#include "texture.hpp"
#include "2iREN/asset/asset_handle.hpp"
#include "2iREN/rhi/resources/graphics_pipeline.hpp"


namespace siren {

/**
 * @brief A Basic PBR material that closely follows the GLTF specs.
 */
class MaterialAsset : public Asset {
public:
    explicit MaterialAsset(const std::string& name) : m_name(name) { }

    // setters

    auto set_base_color(const Rgba& value) noexcept -> void;
    auto set_metallic(f32 value) noexcept -> void;
    auto set_roughness(f32 value) noexcept -> void;
    auto set_base_color_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_metallic_roughness_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_clear_coat(f32 value) noexcept -> void;
    auto set_clear_coat_roughness(f32 value) noexcept -> void;
    auto set_clear_coat_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_clear_coat_roughness_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_clearcoat_normal_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_transmission(f32 value) noexcept -> void;
    auto set_transmission_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_thickness(f32 value) noexcept -> void;
    auto set_attenuation_color(const glm::vec3& value) noexcept -> void;
    auto set_attenuation_distance(f32 value) noexcept -> void;
    auto set_thickness_texture(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_ior(f32 value) noexcept -> void;
    auto set_specular_factor(f32 value) noexcept -> void;
    auto set_specular_color(const glm::vec3& value) noexcept -> void;
    auto set_specular_color_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_specular_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_sheen_color(const glm::vec3& value) noexcept -> void;
    auto set_sheen_roughness(f32 value) noexcept -> void;
    auto set_sheen_color_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_sheen_roughness_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_emissive_strength(f32 value) noexcept -> void;
    auto set_emissive_color(const glm::vec3& value) noexcept -> void;
    auto set_emissive_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_iridescence_factor(f32 value) noexcept -> void;
    auto set_iridescence_ior(f32 value) noexcept -> void;
    auto set_iridescence_min(f32 value) noexcept -> void;
    auto set_iridescence_max(f32 value) noexcept -> void;
    auto set_iridescence_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_iridescence_thickness_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_diffuse_transmission_factor(f32 value) noexcept -> void;
    auto set_diffuse_transmission_color(const glm::vec3& value) noexcept -> void;
    auto set_diffuse_transmission_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_diffuse_transmission_color_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_anisotropy_strength(f32 value) noexcept -> void;
    auto set_anisotropy_rotation(f32 value) noexcept -> void;
    auto set_anisotropy_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_dispersion(f32 value) noexcept -> void;

    auto set_normal_tex(const StrongHandle<Texture>& handle) noexcept -> void;
    auto set_occlusion_tex(const StrongHandle<Texture>& handle) noexcept -> void;

    auto set_alpha_mode(AlphaMode value) noexcept -> void;
    auto set_alpha_cutoff(f32 value) noexcept -> void;
    auto set_double_sided(bool value) noexcept -> void;
    auto set_unlit(bool value) noexcept -> void;

    // getters

    [[nodiscard]] auto base_color() const noexcept -> const Rgba&;
    [[nodiscard]] auto metallic() const noexcept -> f32;
    [[nodiscard]] auto roughness() const noexcept -> f32;
    [[nodiscard]] auto base_color_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto metallic_roughness_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto clear_coat() const noexcept -> f32;
    [[nodiscard]] auto clear_coat_roughness() const noexcept -> f32;
    [[nodiscard]] auto clear_coat_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto clear_coat_roughness_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto clearcoat_normal_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto transmission() const noexcept -> f32;
    [[nodiscard]] auto transmission_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto thickness() const noexcept -> f32;
    [[nodiscard]] auto attenuation_color() const noexcept -> const glm::vec3&;
    [[nodiscard]] auto attenuation_distance() const noexcept -> f32;
    [[nodiscard]] auto thickness_texture() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto ior() const noexcept -> f32;
    [[nodiscard]] auto specular_factor() const noexcept -> f32;
    [[nodiscard]] auto specular_color() const noexcept -> const glm::vec3&;
    [[nodiscard]] auto specular_color_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto specular_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto sheen_color() const noexcept -> const glm::vec3&;
    [[nodiscard]] auto sheen_roughness() const noexcept -> f32;
    [[nodiscard]] auto sheen_color_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto sheen_roughness_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto emissive_strength() const noexcept -> f32;
    [[nodiscard]] auto emissive_color() const noexcept -> const glm::vec3&;
    [[nodiscard]] auto emissive_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto iridescence_factor() const noexcept -> f32;
    [[nodiscard]] auto iridescence_ior() const noexcept -> f32;
    [[nodiscard]] auto iridescence_min() const noexcept -> f32;
    [[nodiscard]] auto iridescence_max() const noexcept -> f32;
    [[nodiscard]] auto iridescence_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto iridescence_thickness_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto diffuse_transmission_factor() const noexcept -> f32;
    [[nodiscard]] auto diffuse_transmission_color() const noexcept -> const glm::vec3&;
    [[nodiscard]] auto diffuse_transmission_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto diffuse_transmission_color_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto anisotropy_strength() const noexcept -> f32;
    [[nodiscard]] auto anisotropy_rotation() const noexcept -> f32;
    [[nodiscard]] auto anisotropy_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto dispersion() const noexcept -> f32;

    [[nodiscard]] auto normal_tex() const noexcept -> const StrongHandle<Texture>&;
    [[nodiscard]] auto occlusion_tex() const noexcept -> const StrongHandle<Texture>&;

    [[nodiscard]] auto alpha_mode() const noexcept -> AlphaMode;
    [[nodiscard]] auto alpha_cutoff() const noexcept -> f32;
    [[nodiscard]] auto double_sided() const noexcept -> bool;
    [[nodiscard]] auto unlit() const noexcept -> bool;

private:
    std::string m_name;

    Rgba m_base_color{ 1.0f };
    f32 m_metallic                                 = 1.0f;
    f32 m_roughness                                = 1.0f;
    StrongHandle<Texture> m_base_color_tex         = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_metallic_roughness_tex = StrongHandle<Texture>::invalid();

    f32 m_clear_coat                                 = 0.0f;
    f32 m_clear_coat_roughness                       = 0.0f;
    StrongHandle<Texture> m_clear_coat_tex           = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_clear_coat_roughness_tex = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_clearcoat_normal_tex     = StrongHandle<Texture>::invalid();

    f32 m_transmission                       = 0.0f;
    StrongHandle<Texture> m_transmission_tex = StrongHandle<Texture>::invalid();
    f32 m_thickness                          = 0.0f;
    glm::vec3 m_attenuation_color{ 1.f };
    f32 m_attenuation_distance                = 0.0f;
    StrongHandle<Texture> m_thickness_texture = StrongHandle<Texture>::invalid();

    f32 m_ior             = 1.5f;
    f32 m_specular_factor = 1.0f;
    glm::vec3 m_specular_color{ 1.f };
    StrongHandle<Texture> m_specular_color_tex = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_specular_tex       = StrongHandle<Texture>::invalid();

    glm::vec3 m_sheen_color{ 0.f };
    f32 m_sheen_roughness                       = 0.0f;
    StrongHandle<Texture> m_sheen_color_tex     = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_sheen_roughness_tex = StrongHandle<Texture>::invalid();

    f32 m_iridescence_factor                          = 0.0f;
    f32 m_iridescence_ior                             = 1.3f;
    f32 m_iridescence_min                             = 100.0f;
    f32 m_iridescence_max                             = 400.0f;
    StrongHandle<Texture> m_iridescence_tex           = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_iridescence_thickness_tex = StrongHandle<Texture>::invalid();

    f32 m_diffuse_transmission_factor = 0.0f;
    glm::vec3 m_diffuse_transmission_color{ 1.f };
    StrongHandle<Texture> m_diffuse_transmission_tex       = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_diffuse_transmission_color_tex = StrongHandle<Texture>::invalid();

    f32 m_anisotropy_strength              = 0.0f;
    f32 m_anisotropy_rotation              = 0.0f;
    StrongHandle<Texture> m_anisotropy_tex = StrongHandle<Texture>::invalid();
    f32 m_dispersion                       = 0.0f;

    StrongHandle<Texture> m_normal_tex    = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_occlusion_tex = StrongHandle<Texture>::invalid();
    StrongHandle<Texture> m_emissive_tex  = StrongHandle<Texture>::invalid();
    glm::vec3 m_emissive_color{ 0.f };
    f32 m_emissive_strength = 1.0f;

    AlphaMode m_alpha_mode = AlphaMode::Opaque;
    f32 m_alpha_cutoff     = 0.5f;
    bool m_double_sided    = false;
    bool m_unlit           = false;
};

} // namespace siren
