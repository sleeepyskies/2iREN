#pragma once

#include <concepts>


namespace siren {

/**
 * @struct Asset
 * @brief Tag struct to identify assets. Should be inherited from by assets.
 */
struct Asset { };

/**
 * @brief Ensures that A is derived from Asset.
 */
template <typename A>
concept IsAsset = std::derived_from<A, Asset>;

} // namespace siren
