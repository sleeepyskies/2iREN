#pragma once

#include <string>
#include <expected>

#include "2iren/asset/asset.hpp"


namespace siren::asset {

class AssetServer;
class LoadContext;

enum class AssetErrorCode { };

using AssetLoadError = std::expected<void, AssetErrorCode>;

template <typename T>
struct LoaderTraits {
    struct Config { };
};

struct AssetLoaderBase {
    virtual ~AssetLoaderBase() = default;
    /** @brief Returns a list of file extensions this loader can load. */
    virtual auto extensions() const -> std::vector<std::string_view> = 0;
};

/**
 * @brief Base class for standard AssetLoaders.
 * See @ref Asset.
 */
template <IsAsset AssetType>
struct AssetLoader : AssetLoaderBase {
    using ConfigType = LoaderTraits<AssetType>::Config;

    ~AssetLoader() override = default;
    /**
     * @brief Loads a new asset into the @ref AssetServer.
     * @param ctx A @ref LoadContext object. Acts as an API to the @ref AssetServer.
     * @param config A config determining how to load the asset.
     * @return Nothing on success, or an error code on fail.
     */
    virtual auto load(LoadContext&& ctx, const ConfigType& config = { }) const -> AssetLoadError = 0;
};

} // namespace siren::asset
