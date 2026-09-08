#pragma once

#include <memory>

#include "2iREN/graphics/device.hpp"
#include "2iREN/utility/log.hpp"

namespace siren {

struct WindowDescriptor;

/// @brief Configuration parameters used to create a @ref Context instance.
struct ContextDescriptor {
    /// @brief Enables additional debug output from the graphics backend.
    bool debug;
    /// @brief Sets the minimum log severity level emitted by the framework.
    log::Level level;
};

/// @brief The entry point and root object of the 2iREN framework.
///
/// A Context initializes the framework's global systems and provides an api
/// for creating other objects.
///
/// A Context should be created once during startup and remain alive for the
/// lifetime of the application.
class Context {
public:
    /// @brief Makes a new framework context.
    [[nodiscard]]
    static auto make(const ContextDescriptor& descriptor) -> Context;

    /// @brief Shutsdown the 2iREN engine.
    ~Context();

    Context(const Context&)            = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&)                 = delete;
    Context& operator=(Context&&)      = delete;

    /// @brief Makes a graphics device.
    [[nodiscard]]
    auto make_device() const -> std::unique_ptr<Device>;

    /// @brief Makes a new window based on the provided descriptor.
    [[nodiscard]]
    auto make_window(const WindowDescriptor& descriptor) const -> Window;

private:
    explicit Context(const ContextDescriptor& descriptor);
};
} // namespace siren
