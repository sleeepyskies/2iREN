#pragma once

#include <string>
#include <expected>

#include "2iren/asset/asset.hpp"


namespace siren {

class AssetServer;
class LoadContext;

/** @brief Error codes possible during asset loading. */
enum class AssetErrorCode {
    /** @brief The asset file could not be located on disk. */
    FileNotFound,
    /** @brief The asset file is somehow corrupted or could not be parsed. */
    InvalidFormat,
    /** @brief The file could be parsed, but was missing some required fields. */
    InvalidSchema,
    /** @brief Some GPU or Driver failure. */
    RuntimeFailed,
    /** @brief Some feature was encountered that 2iren does not support. */
    NotSupported,
    /** @brief Some data has been corrupted. */
    AssetCorrupted
};

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
    virtual auto load(
        LoadContext&& ctx,
        const std::optional<ConfigType> config
    ) const -> AssetLoadError = 0;
};

} // namespace siren
