#pragma once

#include <memory>

#include "2iren/util/log.hpp"
#include "2iren/base.hpp"
#include "fwd.hpp"
#include "2iren/asset/asset_server.hpp"


namespace siren {

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
    [[nodiscard]] auto assets() -> asset::AssetServer&;

private:
    ContextDescriptor m_descriptor;
    asset::AssetServer m_asset_server;
};

} // namespace siren
