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

struct AssetID;

class WeakHandle;

template <typename A>
class StrongHandle;

class AssetPath;

struct AssetLoaderBase;
template <IsAsset A>
struct AssetLoader;

enum class AssetErrorCode;

class AssetServer;
class LoadContext;

} // namespace siren
