#pragma once

#include <glad/gl.h>
#include <unordered_map>
#include <vector>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/backend/opengl/resource_state.hpp"

namespace siren::opengl {

class FramebufferCache {
public:
    explicit FramebufferCache(
        const RenderResourceTable<GLuint, Image, ImageDetails>& images
    ) : m_images{images} { };
    ~FramebufferCache();

    FramebufferCache(const FramebufferCache&)            = delete;
    FramebufferCache& operator=(const FramebufferCache&) = delete;

    [[nodiscard]]
    auto get_create_for(const RenderTarget& target) -> GLuint;

    [[nodiscard]]
    auto get_create_targetless(Extent2 extent) -> GLuint;

    auto invalidate(ImageHandle image) -> void;

private:
    struct Key {
        std::vector<ImageHandle> colors = {};
        ImageHandle depth_stencil = NullHandle;
        auto operator==(const Key&) const -> bool = default;
    };

    struct KeyHasher {
        auto operator()(const Key& key) const -> usize;
    };

    [[nodiscard]]
    auto create_framebuffer(const RenderTarget& target) const -> GLuint;

    [[nodiscard]]
    static auto create_targetless_framebuffer(Extent2 extent) -> GLuint;

    std::unordered_map<Key, GLuint, KeyHasher> m_cache = {};
    std::unordered_map<u64, GLuint> m_targetless_cache = {};
    const RenderResourceTable<GLuint, Image, ImageDetails>& m_images;
};

} // namespace siren::opengl
