#pragma once

#include <Foundation/NSAutoreleasePool.hpp>
#include <Foundation/NSError.hpp>
#include <Foundation/NSSharedPtr.hpp>
#include <Foundation/NSString.hpp>

#include <string>

#include "2iREN/core/assert.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren::metal {

/// @brief Scoped RAII AutoreleasePool wrapper.
struct AutoRelease {
    AutoRelease() : m_pool(NS::AutoreleasePool::alloc()->init()) { }
    ~AutoRelease() {
        m_pool->release();
    }
    NS::AutoreleasePool* m_pool;
};

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

/// @brief Creates a new NS::String with UTF8 encoding.
/// @note Cannot take a string_view here as NS::String expects a null
/// terminated string.
constexpr auto utf8_string(const std::string& str) -> NS::SharedPtr<NS::String> {
    return retain_ptr(
        NS::String::alloc()->init(str.data(), NS::StringEncoding::UTF8StringEncoding)
    );
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
    if (object.get() == nullptr) {
        PANIC("failed to create metal resource {}. error: {}", typename_of<T>(), error_msg(err));
    } else {
        err = nullptr;
    }
}

} // namespace siren::metal
