#pragma once 

#include <vector>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/backend/opengl/opengl.hpp"
#include "2iREN/graphics/commands.hpp"

namespace siren {

class FramebufferCache {
public:
    explicit FramebufferCache(
        const RenderResourceTable<GLuint, Image, GlImageDetails>& image_table
    ) : m_image_table{image_table} { };

    [[nodiscard]]
    auto get_create_for(const RenderTarget& target) -> GLuint;

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
    std::unordered_map<Key, GLuint, KeyHasher> m_cache = {};
    const RenderResourceTable<GLuint, Image, GlImageDetails>& m_image_table;
};

auto FramebufferCache::get_create_for(const RenderTarget& target) -> GLuint {
    // first search cache
    const Key key{
        .colors = target.colors
            | std::views::transform(&ColorAttachment::image)
            | std::ranges::to<std::vector>(),
        .depth_stencil = target.depth_stencil.transform(
                                                 [](auto a) { return a.image; }
        ).value_or(NullHandle),
    };
    if (const auto it = m_cache.find(key); it != m_cache.end()) {
        return it->second;
    }

    // otherwise create a new framebuffer
    const auto fb = create_framebuffer(target);
    m_cache[key]  = fb;
    return fb;
}

auto FramebufferCache::Hasher::operator()(const Key& key) const -> usize {
    // ty mr chatgpt, idk nun bout this
    usize hash   = 0;
    auto combine = [&hash](const usize value) {
        hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    };
    for (const auto image : key.colors)
        combine(image.hash());
    if (key.depth_stencil.is_valid()) {
        combine(key.depth_stencil.hash());
    }
    return hash;
}

auto FramebufferCache::create_framebuffer(const RenderTarget& target) const -> GLuint {
    GLuint framebuffer;
    glCreateFramebuffers(1, &framebuffer);

    for (const auto [index, attachment] : std::views::enumerate(target.colors)) {
        const auto image_id = m_image_table.fetch(attachment.image);
        glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0 + index, image_id, 0);
    }

    if (target.depth_stencil.has_value()) {
        const auto image_id = m_image_table.fetch(target.depth_stencil->image);
        const auto type     = m_image_table.details(target.depth_stencil->image).descriptor.format;
        switch (type) {
            case ImageFormat::Depth32f:
                glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, image_id, 0);
                break;
            case ImageFormat::Depth24Stencil8:
                glNamedFramebufferTexture(framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, image_id, 0);
                break;
            default:
                PANIC(
                    "Depth/Stencil buffer must have either Depth32f or "
                    "Depth24Stencil8 format"
                );
        }
    }

    // makes the buffers drawable
    std::vector<GLenum> draw_buffers;
    draw_buffers.reserve(target.colors.size());
    for (const usize index : range(target.colors.size())) {
        draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + index);
    }
    glNamedFramebufferDrawBuffers(framebuffer, draw_buffers.size(), draw_buffers.data());

    ASSERT(
        glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "Framebuffer could not be created."
    );

    return framebuffer;
}

}
