#include "framebuffer_cache.hpp"

#include <algorithm>
#include <ranges>

#include "2iREN/graphics/commands.hpp"
#include "2iREN/utility/hash_combine.hpp"

namespace siren::opengl {

namespace {

constexpr auto targetless_key(const Extent2 extent) -> u64 {
    return (static_cast<u64>(extent.x) << 32) | extent.y;
}

} // namespace

FramebufferCache::~FramebufferCache() {
    for (const auto framebuffer : m_cache | std::views::values) {
        glDeleteFramebuffers(1, &framebuffer);
    }
    for (const auto framebuffer : m_targetless_cache | std::views::values) {
        glDeleteFramebuffers(1, &framebuffer);
    }
}

auto FramebufferCache::get_create_for(const RenderTarget& target) -> GLuint {
    // first search cache
    const auto key = Key{
        .colors = target.colors
            | std::views::transform(&TargetColorAttachment::image)
            | std::ranges::to<std::vector>(),

        .depth_stencil = target.depth_stencil
            .transform([](auto a) { return a.image; })
            .value_or(NullHandle),
    };
    if (const auto it = m_cache.find(key); it != m_cache.end()) {
        return it->second;
    }

    // otherwise create a new framebuffer
    const auto fb = create_framebuffer(target);
    m_cache[key]  = fb;
    return fb;
}

auto FramebufferCache::get_create_targetless(const Extent2 extent) -> GLuint {
    const auto key = targetless_key(extent);
    if (const auto it = m_targetless_cache.find(key); it != m_targetless_cache.end()) {
        return it->second;
    }

    const auto framebuffer = create_targetless_framebuffer(extent);
    m_targetless_cache.emplace(key, framebuffer);
    return framebuffer;
}

auto FramebufferCache::invalidate(const ImageHandle image) -> void {
    for (auto it = m_cache.begin(); it != m_cache.end();) {
        const auto references_image = it->first.depth_stencil == image
            or std::ranges::find(it->first.colors, image) != it->first.colors.end();

        if (!references_image) {
            ++it;
            continue;
        }

        glDeleteFramebuffers(1, &it->second);
        it = m_cache.erase(it);
    }
}

auto FramebufferCache::KeyHasher::operator()(const Key& key) const -> usize {
    usize hash   = 0;
    for (const auto image : key.colors) {
        hash_combine(hash, image.hash());
    }
    hash_combine(hash, key.depth_stencil.hash());
    return hash;
}

auto FramebufferCache::create_framebuffer(const RenderTarget& target) const -> GLuint {
    GLuint framebuffer;
    glCreateFramebuffers(1, &framebuffer);

    for (const auto [index, attachment] : std::views::enumerate(target.colors)) {
        ASSERT(
            !m_images.details(attachment.image).default_framebuffer,
            "the default framebuffer cannot be attached to another framebuffer"
        );
        const auto image_id = m_images.fetch(attachment.image);
        glNamedFramebufferTexture(
            framebuffer,
            static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index),
            image_id,
            0
        );
    }

    if (target.depth_stencil.has_value()) {
        ASSERT(
            !m_images.details(target.depth_stencil->image).default_framebuffer,
            "the default framebuffer cannot be attached to another framebuffer"
        );
        const auto image_id = m_images.fetch(target.depth_stencil->image);
        const auto type     = m_images.details(target.depth_stencil->image).descriptor.format;
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
        draw_buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index));
    }

    if (draw_buffers.empty()) {
        glNamedFramebufferDrawBuffer(framebuffer, GL_NONE);
        glNamedFramebufferReadBuffer(framebuffer, GL_NONE);
    } else {
        glNamedFramebufferDrawBuffers(
            framebuffer,
            static_cast<GLsizei>(draw_buffers.size()),
            draw_buffers.data()
        );
    }

    ASSERT(
        glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "framebuffer could not be created"
    );

    return framebuffer;
}

auto FramebufferCache::create_targetless_framebuffer(const Extent2 extent) -> GLuint {
    GLuint framebuffer;
    glCreateFramebuffers(1, &framebuffer);

    glNamedFramebufferParameteri(
        framebuffer,
        GL_FRAMEBUFFER_DEFAULT_WIDTH,
        static_cast<GLint>(extent.x)
    );
    glNamedFramebufferParameteri(
        framebuffer,
        GL_FRAMEBUFFER_DEFAULT_HEIGHT,
        static_cast<GLint>(extent.y)
    );
    glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_LAYERS, 1);
    glNamedFramebufferDrawBuffer(framebuffer, GL_NONE);
    glNamedFramebufferReadBuffer(framebuffer, GL_NONE);

    ASSERT(
        glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "targetless framebuffer could not be created"
    );

    return framebuffer;
}

} // namespace siren::opengl
