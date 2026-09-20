#pragma once

#include <Foundation/NSAutoreleasePool.hpp>
#include <Foundation/NSError.hpp>
#include <Foundation/NSSharedPtr.hpp>
#include <Foundation/NSString.hpp>

#include <Metal/MTLCommandBuffer.hpp>
#include <concepts>
#include <string>

#include "2iREN/core/assert.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren::metal {

#define AUTORELEASE_STR_CONCAT__(a, b) a##b
#define AUTORELEASE_STR_CONCAT_(a, b) AUTORELEASE_STR_CONCAT__(a, b)

/// #brief Provides a way to wrap a scope with an auto release pool. Mirrors the objective c way of
/// doing things and avoids manual release.
#define AUTORELEASE                                                                                \
    if (const auto AUTORELEASE_STR_CONCAT_(_autorelease_, __LINE__) =                              \
            NS::TransferPtr(NS::AutoreleasePool::alloc()->init());                                 \
        true) [[likely]]

/// @brief Creates a new NS::SharedPtr from a raw pointer.
/// @note This does *not* increase the retain count.
template <typename T>
constexpr auto transfer_ptr(T* ptr) -> NS::SharedPtr<T> {
    ASSERT_NOT_NULL(ptr, "call to transfer_ptr failed.");
    return NS::TransferPtr(ptr);
}

/// @brief Creates a new NS::SharedPtr from a raw pointer.
/// @note This *does* increase the retain count.
template <typename T>
constexpr auto retain_ptr(T* ptr) -> NS::SharedPtr<T> {
    ASSERT_NOT_NULL(ptr, "call to retain_ptr failed.");
    return NS::RetainPtr(ptr);
}

/// @brief Creates a new autoreleased NS::String with UTF8 encoding.
/// @note The passed in string must be null terminated.
inline auto utf8_string(const std::string& str) -> NS::String* {
    return NS::String::string(str.c_str(), NS::StringEncoding::UTF8StringEncoding);
}

/// @brief Returns the error message contained within an error.
/// @note Assumes that an error occurred, and thus if nullptr is passed in, it
/// is assumed to be an unkonwn error.
constexpr auto error_msg(NS::Error* err) -> std::string {
    if (!err) {
        return "unknown metal error.";
    }

    std::string out;

    if (err->localizedDescription()) {
        out += "\n";
        out += "description: ";
        out += err->localizedDescription()->cString(NS::UTF8StringEncoding);
        out += "\n";
    }

    if (err->localizedFailureReason()) {
        out += "reason: ";
        out += err->localizedFailureReason()->cString(NS::UTF8StringEncoding);
        out += "\n";
    }

    if (err->localizedRecoverySuggestion()) {
        out += "suggestion: ";
        out += err->localizedRecoverySuggestion()->cString(NS::UTF8StringEncoding);
    }

    return out.empty() ? "unknown metal error." : out;
}

/// @brief Checks if an erroy occurred during making objcect.
template <typename T>
constexpr auto check_error(T* object, NS::Error* err) -> void {
    if (object == nullptr) {
        PANIC("failed to create metal resource {}. error: {}", typename_of<T>(), error_msg(err));
    } else {
        err = nullptr;
    }
}

/// @brief Checks if an erroy occurred during making objcect.
template <typename T>
constexpr auto check_error(NS::SharedPtr<T> object, NS::Error* err) -> void {
    check_error(object.get(), err);
}

/// @brief Ensures a metal resource has a label field.
template <typename R>
concept HasLabel = requires(R& r, NS::String* str) { r.setLabel(str); };

/// @brief Helper method to set a resources label, if there is a label and the resource can hold a
/// label.
template <HasLabel R>
constexpr auto set_label(const NS::SharedPtr<R>& resource, const Label& label) {
    if (!resource || !label) {
        return;
    }
    resource->setLabel(utf8_string(label->c_str()));
}

/// @brief Checks the command buffers stateus of committing.
inline auto check(MTL::CommandBuffer* cmdbuffer) -> void {
    ASSERT(
        cmdbuffer->status() != MTL::CommandBufferStatusError,
        "command buffer failed during execution with error {}",
        error_msg(cmdbuffer->error())
    );
}
} // namespace siren::metal
