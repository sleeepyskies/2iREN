#pragma once

#include <memory>

#include "2iren/util/log.hpp"
#include "2iren/base.hpp"
#include "fwd.hpp"
#include "2iren/asset/asset_server.hpp"


namespace siren {

// todo: maybe Context should be a struct holding AssetServer and Device and eventually Renderer?
// so probable like a free function to create a siren context, with a ContextDescriptor,
// then this context is a simple struct with all other objects as fields ig, so
// create_context({ .server_config = std::nullopt, .render_config = { ... }, .debuug = true, })...
// then ctx.asset_server.load(), ctx.renderer.begin(), ctx.device.create_buffer() etc

/**
 * @brief Parameters used to instantiate the context.
 */
struct ContextDescriptor {
    /** @brief Whether to enable additional graphics API debug information. */
    bool debug;
    /** @brief The desired log level. */
    log::Level level;
    /** @brief Which backend graphics API to select. */
    Backend backend;
};

class Context {
public:
    explicit Context(const ContextDescriptor& descriptor);

    Context(const Context&)            = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&)                 = delete;
    Context& operator=(Context&&)      = delete;

    [[nodiscard]] auto create_device(const Window& window) const -> std::unique_ptr<Device>;

private:
    ContextDescriptor m_descriptor;
};

} // namespace siren
