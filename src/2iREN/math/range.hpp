#pragma once

namespace siren {

/// @brief Represents an inlusive range between an inlusive start and an
/// exclusive end.
template <typename T>
struct Range {
    /// @brief The inlusive begin.
    T begin;
    /// @brief The exclusive end.
    T end;
};

} // namespace siren
