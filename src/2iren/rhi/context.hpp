#pragma once

#include <memory>

#include "2iren/base.hpp"
#include "2iren/util/log.hpp"
#include "device.hpp"
#include "fwd.hpp"

namespace siren {
/**
 * @brief Configuration parameters used to create a @ref Context instance.
 */
struct ContextDescriptor {
    /**
     * @brief Enables additional debug output from the graphics backend.
     *
     * When enabled, graphics API validation and debug messages may be emitted.
     */
    bool debug;

    /**
     * @brief Sets the minimum log severity level emitted by the framework.
     */
    log::Level level;

    /**
     * @brief Selects the graphics backend used by the framework.
     */
    Backend backend;
};

/**
 * @class Context
 * @brief The entry point and root object of the 2iren framework.
 *
 * A Context initializes the framework's global systems and provides factory
 * functions for creating major objects.
 *
 * A Context should typically be created once during application startup and
 * remain alive for the lifetime of the application.
 */
class Context {
public:
    /**
     * @brief Creates a new framework context.
     *
     * @note This should only be called once!
     * @param descriptor Configuration parameters controlling initialization.
     * @return A fully initialized Context instance.
     */
    static auto create(const ContextDescriptor& descriptor) -> Context;

    /**
     * @brief Destroys the context and releases all owned resources.
     */
    ~Context();

    Context(const Context&)            = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&)                 = delete;
    Context& operator=(Context&&)      = delete;

    /**
     * @brief Creates a graphics device.
     */
    [[nodiscard]] auto create_device(const DeviceDescriptor& descriptor) const -> std::unique_ptr<Device>;

    /**
     * @brief Creates a new window based on the provided descriptor.
     */
    [[nodiscard]] auto create_window(const WindowDescriptor& descriptor) const -> Window;

private:
    explicit Context(const ContextDescriptor& descriptor);
    ContextDescriptor m_descriptor;
};
} // namespace siren
