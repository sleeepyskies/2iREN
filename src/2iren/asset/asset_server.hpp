#pragma once

#include <any>
#include <unordered_map>
#include <unordered_set>

#include "asset_handle.hpp"
#include "asset_id.hpp"
#include "asset_utils.hpp"
#include "asset_pool.hpp"
#include "asset_loader.hpp"
#include "2iren/sync/rw_lock.hpp"
#include "2iren/sync/thread_pool.hpp"
#include "2iren/util/log.hpp"


namespace siren {

/// todo:
///     maybe the asset server shouldn't even have access to the device?
///     we could just load CPU version of the asset. This data could be used
///     to create individual GPU objects via device, but having server call
///     device directly is not great imo... but then for this there should
///     exist a Renderer that simplifies Asset -> GPU object creation

class Device;

/**
 * @brief Represents the loading status of an asset.
 */
enum class LoadStatus {
    /** @brief The asset is not loaded, and loading hasn't begun. */
    NotLoaded,
    /** @brief The asset is currently being loaded by an AssetLoader. */
    Loading,
    /** @brief The asset was successfully loaded. */
    Loaded,
    /** @brief There was an error while loading the asset. */
    Fail,
};

/** @brief Simple state machine utility class for managing LoadStatus. */
class LoadState {
public:
    [[nodiscard]] constexpr auto get_main() const noexcept -> LoadStatus { return m_main; }
    constexpr auto set_main(const LoadStatus status) noexcept -> void {
        if (m_main == LoadStatus::Fail || m_dependencies == LoadStatus::Fail) { return; }
        if (status == LoadStatus::NotLoaded) { return; }
        if (status == LoadStatus::Fail) { m_dependencies = status; }
        m_main = status;
    }

    [[nodiscard]] constexpr auto get_deps() const noexcept -> LoadStatus { return m_dependencies; }

    constexpr auto set_deps(const LoadStatus status) noexcept -> void {
        if (m_main == LoadStatus::Fail || m_dependencies == LoadStatus::Fail) { return; }
        if (status == LoadStatus::NotLoaded) { return; }
        if (status == LoadStatus::Fail) { m_main = status; }
        m_dependencies = status;
    }

    [[nodiscard]]
    constexpr auto is_ready() const noexcept -> bool {
        return m_main == m_dependencies && m_main == LoadStatus::Loaded;
    }

    [[nodiscard]]
    constexpr auto failed() const noexcept -> bool {
        return m_main == LoadStatus::Fail || m_dependencies == LoadStatus::Fail;
    }

private:
    LoadStatus m_main         = LoadStatus::NotLoaded;
    LoadStatus m_dependencies = LoadStatus::NotLoaded;
};

class AssetServer {
    using TypeID = AssetID::TypeID;

    struct AssetInfo {
        /// @brief The path of the asset (if it was loaded from disk).
        /// @todo add this!
        // AssetPath path;
        /** @brief The main asset. */
        WeakHandle weak_handle;
        /** @brief The load state of this asset and its dependencies. */
        LoadState load_state;
        /** @brief Any labeled (aka embedded) assets of the main asset. */
        std::unordered_map<std::string, WeakHandle> labeled_deps;
        /** @brief Any external (aka external files) dependencies of this main asset. */
        std::unordered_set<WeakHandle> dependencies;
        /** @brief All assets that are waiting for this asset to finish loading. */
        std::unordered_set<WeakHandle> dependents;
    };

    struct Loaders {
        /** @brief Main loader storage. */
        std::vector<std::unique_ptr<AssetLoaderBase>> loaders;
        /** @brief Cached loaders based on their accepted file extensions. */
        std::unordered_map<std::string, AssetLoaderBase*> ext_to_loader;
    };

    struct AssetServerData {
        /** @brief General data on assets (dep tree, load status). */
        RwLock<std::unordered_map<HashedString, AssetInfo>> asset_infos;
        /** @brief Main storage for asset data. */
        RwLock<std::unordered_map<TypeID, std::unique_ptr<AssetPoolBase>>> storage{ };
        /**
         * @brief All active loaders.
         * @note The reason we do not use any sync primitives here, is that Loaders are
         * in principle stateless. Furthermore, we assume all loaders are registered at engine init.
         * If this is not the case, and loaders are added at runtime, there can be issues.
         */
        Loaders loaders;
        /**
         * @brief Cache of at most a single default handle per asset type.
         * @todo do we have to use std::any? not great imo, but @ref WeakHandle is not ref counted,
         * but StrongHandle is not type erased such that we can store in a container....
         */
        RwLock<std::unordered_map<TypeID, std::any>> default_handles{ };
    };

public:
    explicit AssetServer(Device& device);

    /**
     * @brief Attempts to find and return the asset that the provided handle points to.
     * If the asset cannot be found or is still being loaded, this will return a nullptr.
     * @tparam A The specific asset type.
     * @param handle A handle to the asset.
     * @return The asset if it can be found on the server, otherwise a nullptr.
     */
    template <IsAsset A>
    [[nodiscard]] auto get(const StrongHandle<A> handle) -> A* {
        if (!handle.is_valid()) {
            log::debug("Could not get asset {}, as the handle is invalid.", handle);
            return nullptr;
        }

        if (!is_loaded_with_dependencies(handle)) {
            log::debug("Could not get asset {}, as at least of of its dependencies is still loading.", handle);
            return nullptr;
        }

        return m_data.storage.run(
            [&handle] (const std::unordered_map<TypeID, std::unique_ptr<AssetPoolBase>>& storage) -> A*{
                const auto it = storage.find(AssetID::type_id<A>());
                if (it == storage.end()) {
                    log::debug("{} asset storage does not exist, cannot get asset.", typename_of<A>(), handle);
                    return nullptr;
                }
                auto* pool = static_cast<AssetPool<A>*>(it->second.get());
                return pool->fetch(handle.id());
            }
        );
    }

    /**
     * @brief Attempts to find and return the asset that the provided handle points to.
     * @warning Crashes on failure, use with caution.
     * @tparam A The specific asset type.
     * @param handle A handle to the asset.
     * @return The asset if it can be found on the server, otherwise a nullptr.
     */
    template <IsAsset A>
    [[nodiscard]] auto get_unsafe(const StrongHandle<A> handle) const -> const A& {
        auto* asset = get(handle);
        ASSERT(asset, std::format("Failed to get asset from handle {}", handle));
        return *asset;
    }

    /**
     * @brief Attempts to find and return the asset that the provided handle points to.
     * @warning Crashes on failure, use with caution.
     * @tparam A The specific asset type.
     * @param handle A handle to the asset.
     * @return The asset if it can be found on the server, otherwise a nullptr.
     */
    template <IsAsset A>
    [[nodiscard]] auto get_unsafe(const StrongHandle<A> handle) -> A& {
        auto* asset = get(handle);
        ASSERT(asset, std::format("Failed to get asset from handle {}", handle));
        return *asset;
    }

    /**
     * @brief Loads an asset from disk, as well as recursively loading all of its dependencies.
     * @tparam A The asset type of the to be loaded asset.
     * @param path The string asset path of the asset to load. @see AssetPath for format options.
     * @param config An optional configuration object with influences how to load the asset.
     * @return A handle to the loaded asset.
     */
    template <IsAsset A>
    [[nodiscard]] auto load(
        const std::string& path,
        std::optional<typename AssetLoader<A>::ConfigType> config = std::nullopt
    ) -> StrongHandle<A> { return load<A>(AssetPath::parse(path), config); }

    /**
     * @brief Loads an asset from disk, as well as recursively loading all of its dependencies.
     * @tparam A The asset type of the to be loaded asset.
     * @param path The @ref AssetPath of the asset to load.
     * @param config An optional configuration object with influences how to load the asset.
     * @return A handle to the loaded asset.
     */
    template <IsAsset A>
    [[nodiscard]] auto load(
        const AssetPath& path,
        std::optional<typename AssetLoader<A>::ConfigType> config = std::nullopt
    ) -> StrongHandle<A>;

    /**
     * @brief Directly adds the provided asset into storage, if a pool exists for its type.
     * @tparam A The type of the asset being added.
     * @param asset The asset to add.
     * @param path An optional path parameter. Useful if adding as asset that was loaded external to the server.
     * @return A @ref StrongHandle referencing the newly added asset.
     */
    template <IsAsset A>
    [[nodiscard]] auto add(
        std::unique_ptr<A>&& asset,
        const AssetPath& path = AssetPath::invalid()
    ) -> StrongHandle<A> {
        ensure_asset_registered<A>();
        log::trace("Attempting to add new asset of type {}", typename_of<A>());

        return m_data.storage.run_exclusive(
            [asset = std::move(asset), &path] (auto& storage) mutable {
                const auto it = storage.find(AssetID::type_id<A>());
                if (it == storage.end()) {
                    log::error("Could not find an appropriate asset pool for type {}", typename_of<A>());
                    return StrongHandle<A>::invalid();
                }
                auto& pool = pool_cast<A>(it->second.get());
                const AssetID id = pool.add(std::forward<std::unique_ptr<A>>(asset));
                // todo: can we handle non disk assets better, for example a UUID
                return StrongHandle<A>{ id, pool, path};
            }
        );
    }

    /**
     * @brief Shallow checks if this asset is loaded. The status of any dependencies is ignored.
     * @param handle The handle to check the status of.
     * @return True if this asset is loaded ignoring dependencies, false otherwise.
     */
    template <IsAsset A>
    auto is_loaded(const StrongHandle<A>& handle) -> bool {
        return m_data.asset_infos.run(
            [&handle] (const std::unordered_map<HashedString, AssetInfo>& infos){
                const auto it = infos.find(handle.relative_path().hashed_string());
                if (it == infos.end()) { return false; }
                return it->second.load_state.get_main() == LoadStatus::Loaded;
            }
        );
    }

    /**
     * @brief Recursively checks if this asset is loaded. The status of all dependencies is also checked.
     * @param handle The handle to check the status of.
     * @return True if this asset is loaded including dependencies, false otherwise.
     */
    template <IsAsset A>
    auto is_loaded_with_dependencies(const StrongHandle<A>& handle) -> bool {
        // todo: this fails for subassets, aka any asset with no path, as it then has no entry in asset_infos
        return m_data.asset_infos.run(
            [&handle] (const std::unordered_map<HashedString, AssetInfo>& infos){
                const auto it = infos.find(handle.path().hashed_string());
                if (it == infos.end()) { return false; }
                return it->second.load_state.is_ready();
            }
        );
    }

    /**
     * @brief Registers a loader with the asset server. No assets can be
     * loaded from disk until an appropriate loader has been registered.
     * @param loader
     */
    auto register_loader(std::unique_ptr<AssetLoaderBase> loader) -> void {
        // todo: do a check for loaders here that already exists?
        // todo: should ig also auto register the type here
        auto& loaders         = m_data.loaders;
        const auto loader_ptr = loaders.loaders.emplace_back(std::move(loader)).get();
        for (const auto& ext : loader_ptr->extensions()) {
            loaders.ext_to_loader.insert({ std::string(ext), loader_ptr });
        }
    }

    /**
     * @brief Fetches a handle to the default asset of type A, iff present.
     * @tparam A The asset type to fetch a handle to the default asset for.
     */
    template <IsAsset A>
    auto fetch_default() const -> StrongHandle<A> {
        return m_data.default_handles.run([] (const std::unordered_map<TypeID, std::any>& default_handles){
            const auto it = default_handles.find(AssetID::type_id<A>());
            if (it == default_handles.end()) {
                log::error("There exists no default for asset type {}, but it was requested.", typename_of<A>());
                return StrongHandle<A>::invalid();
            }
            return std::any_cast<StrongHandle<A>>(it->second);
        });
    }

    /**
     * @brief Registers a default asset instance for the type A. If one is already present,
     * it will be overwritten.
     * @tparam A The asset type to register a new default asset for.
     */
    template <IsAsset A>
    auto register_default(std::unique_ptr<A>&& asset) -> void {
        StrongHandle<A> handle = add(std::move(asset));
        log::info("Registering a new default for type {}. Default handle: {}", typename_of<A>(), handle);
        m_data.default_handles.run_exclusive(
            [handle] (std::unordered_map<TypeID, std::any>& default_handles){
                default_handles[AssetID::type_id<A>()] = std::any{ handle };
            }
        );
    }

private:
    friend class LoadContext; // Need access to create the dependency tree.

    /**
     * @brief Ensures that the asset type has a registered storage block.
     * @tparam A The asset type to register.
     * @note This may lock m_data.storage, and will read from it, so make sure there is no lock on this yet!
     */
    template <IsAsset A>
    auto ensure_asset_registered() {
        const auto tid = AssetID::type_id<A>();

        auto contains = m_data.storage.run(
            [tid] (const auto& storage) -> bool{
                return storage.contains(tid);
            }
        );

        if (contains) { return; }

        log::debug("Registering a new asset type: {}", typename_of<A>());
        m_data.storage.run_exclusive(
            [tid] (auto& storage){
                (void)storage.emplace(tid, std::make_unique<AssetPool<A>>()).first;
            }
        );
    }

    /**
     * Searches the cache for a handle to the asset based on its path.
     * @tparam A The asset type to search for.
     * @param path The path of the asset used as a cache key.
     * @return An optional handle to the asset.
     */
    template <IsAsset A>
    auto search_cache(const AssetPath& path) -> std::optional<StrongHandle<A>> {
        return m_data.asset_infos.run([path] (const auto& asset_infos) -> std::optional<StrongHandle<A>>{
            // search cache
            auto ait = asset_infos.find(path.hashed_string());

            // nothing inside of cache
            if (ait == asset_infos.end()) {
                return std::nullopt;
            }

            // there is no label, this is the main asset.
            if (!path.label()) {
                return make_strong<A>(ait->second.weak_handle);
            }

            // this is a labeled sub asset
            const auto lit = ait->second.labeled_deps.find(path.label().value());
            if (lit != ait->second.labeled_deps.end()) {
                return make_strong<A>(lit->second);
            }

            return std::nullopt;
        });
    }

    /**
     * Attempts to fetch a suitable loader for the asset type and file type.
     * @tparam A The type of asset the loader is for.
     * @param ext The extension the loader should handle.
     * @return A pointer to a loader, or nullptr in case on does not exist.
     */
    template <IsAsset A>
    auto fetch_loader(const std::string& ext) -> AssetLoader<A>* {
        const auto it = m_data.loaders.ext_to_loader.find(ext);
        if (it == m_data.loaders.ext_to_loader.end()) {
            return nullptr;
        }
        return dynamic_cast<AssetLoader<A>*>(it->second);
    }

    /** @brief The underlying data of the AssetServer. */
    AssetServerData m_data;
    /** @brief Handle to the device used for creation of certain render objects. */
    Device& m_device;
};

/**
 * @brief LoadContext serves as the AssetServer API for AssetLoaders. It handles registering sub assets, as well as
 * tracking asset dependencies.
 */
class LoadContext {
public:
    LoadContext(AssetServer& server, const AssetPath& path, const WeakHandle& handle, Device& device)
        : m_server(server), m_handle(handle), m_path(path), m_device(device) { }

    template <IsAsset A>
    [[nodiscard]] auto add_labeled_asset(const std::string& label, std::unique_ptr<A>&& asset) -> StrongHandle<A> {
        const StrongHandle<A> handle = m_server.add<A>(std::move(asset), m_path);

        // the asset_info should exist already, if it doesn't please crash, something went wrong :D
        auto asset_infos = m_server.m_data.asset_infos.write();
        auto& asset_info = asset_infos->at(path().hashed_string());

        // some issue with adding the new labeled asset
        if (!handle.is_valid()) {
            asset_info.load_state.set_main(LoadStatus::Fail);
            poison_dependents(m_handle, *asset_infos);
            return handle;
        }

        asset_info.labeled_deps.emplace(label, make_weak(handle));
        return handle;
    }

    template <IsAsset A>
    [[nodiscard]]
    auto load_external_asset(
        const std::string& asset_path,
        std::optional<typename AssetLoader<A>::ConfigType> config = std::nullopt
    ) -> StrongHandle<A> { return load_external_asset<A>(AssetPath::parse(asset_path), std::move(config)); }


    template <IsAsset A>
    [[nodiscard]] auto load_external_asset(
        const AssetPath& asset_path,
        std::optional<typename AssetLoader<A>::ConfigType> config = std::nullopt
    ) -> StrongHandle<A> {
        const StrongHandle<A> handle = m_server.load<A>(asset_path, std::move(config));

        // the asset_info should exist already, if it doesn't please crash, something went wrong :D
        auto asset_infos          = m_server.m_data.asset_infos.write();
        auto& main_asset_info     = asset_infos->at(path().hashed_string());
        auto& external_asset_info = asset_infos->at(asset_path.hashed_string());

        // some issue with adding the new external asset
        if (!handle.is_valid()) {
            external_asset_info.load_state.set_main(LoadStatus::Fail);
            poison_dependents(external_asset_info.weak_handle, *asset_infos);
            return handle;
        }

        main_asset_info.dependencies.emplace(make_weak(handle));
        external_asset_info.dependents.emplace(m_handle);
        return handle;
    }

    template <IsAsset A>
    auto finish(std::unique_ptr<A>&& asset) -> void {
        auto asset_infos = m_server.m_data.asset_infos.write();
        auto storage     = m_server.m_data.storage.write();

        // the asset_info should exist already, if it doesn't please crash, something went wrong :D
        const HashedString key = path().hashed_string(); // Store in a variable first
        auto& asset_info       = asset_infos->at(key);
        asset_info.load_state.set_main(LoadStatus::Loaded);

        // if we have no pending sub assets, work up the tree.
        notify_dependents(m_handle, *asset_infos);

        auto& pool = *dynamic_cast<AssetPool<A>*>(storage->at(AssetID::type_id<A>()).get());
        pool.link(m_handle.id(), std::move(asset));
    }

    template <IsAsset A>
    auto fetch_default() -> StrongHandle<A> { return m_server.fetch_default<A>(); }

    /** @brief Returns the @ref AssetPath this LoadContext was created for. */
    [[nodiscard]] constexpr auto path() const noexcept -> const AssetPath& { return m_handle.path(); }
    /** @brief Returns the @ref Device this LoadContext is using. */
    [[nodiscard]] constexpr auto device() noexcept -> Device& { return m_device; }
    /** @brief Returns the @ref WeakHandle this LoadContext was made to load an asset for. */
    [[nodiscard]] constexpr auto handle() noexcept -> WeakHandle { return m_handle; }

private:
    auto notify_dependents(
        const WeakHandle& handle,
        std::unordered_map<HashedString, AssetServer::AssetInfo>& asset_infos
    ) const -> void {
        auto& asset_info = asset_infos.at(handle.path().hashed_string());

        // still waiting, do not notify parents yet
        if (!asset_info.dependencies.empty()) { return; }

        asset_info.load_state.set_main(LoadStatus::Loaded);
        asset_info.load_state.set_deps(LoadStatus::Loaded);

        // totally done, work up the tree
        for (auto& parent : asset_info.dependents) {
            auto& parent_info = asset_infos.at(parent.path().hashed_string());
            parent_info.dependencies.erase(handle);
            notify_dependents(parent, asset_infos);
        }
    }

    auto poison_dependents(
        const WeakHandle& handle,
        std::unordered_map<HashedString, AssetServer::AssetInfo>& asset_infos
    ) const -> void {
        auto& asset_info = asset_infos.at(handle.path().hashed_string());

        asset_info.load_state.set_main(LoadStatus::Fail);
        asset_info.load_state.set_deps(LoadStatus::Fail);

        for (auto& parent : asset_info.dependents) {
            poison_dependents(parent, asset_infos);
        }

        // do not load again, as we can cache the failed load,
        // so we don't try to load and fail again
    }

    AssetServer& m_server; ///< @brief The asset server to load into.
    WeakHandle m_handle;   ///< @brief The handle to the main asset.
    AssetPath m_path;      ///< @brief The filepath to the asset to load.
    Device& m_device;      ///< @brief A reference to the device for the
};

// general process here goes:
// 1. ensure asset storage exists
// 2. if cached, return handle from cache
// 3. if no loader exists, return invalid handle
// 4. spawn new load task
template <IsAsset A>
[[nodiscard]] auto AssetServer::load(
    const AssetPath& path,
    std::optional<typename AssetLoader<A>::ConfigType> config
) -> StrongHandle<A> {
    ensure_asset_registered<A>();
    log::debug("Loading new asset from path {}", path);

    // generate new or fetch weak handle from cache
    if (const auto cached = search_cache<A>(path); cached) {
        return *cached;
    }

    // check if loader exists
    auto loader = fetch_loader<A>(path.extension());
    if (!loader) {
        log::warn("Could not load asset of type {}, as there exists no loader for it.", typename_of<A>());
        return StrongHandle<A>::invalid();
    }

    // generate a new handle
    const auto weak_handle = m_data.storage.run_exclusive([path](auto& storage) -> WeakHandle {
        const auto it = storage.find(AssetID::type_id<A>());
        auto pool = static_cast<AssetPool<A>*>(it->second.get());
        return WeakHandle{
            pool->reserve(),
            pool,
            path
        };
    });

    // todo: i think we should clean up this AssetInfo entry in the case where loading fails.
    log::trace("Asset {} does not exist in cache, attempting to load from disk.", weak_handle);
    m_data.asset_infos.run_exclusive([weak_handle, path](auto& asset_infos){
        asset_infos.emplace(
            path.hashed_string(),
            AssetInfo{
                // .path = {},
                .weak_handle = weak_handle,
                .load_state = { },
                .labeled_deps = { },
                .dependencies = { },
                .dependents = { },
            }
        );
    });

    // spawn new loading task
    ThreadPool::get().spawn_detached(
        [this, path, loader, weak_handle, config = std::move(config)] mutable {
            const auto result = loader->load(LoadContext{ *this, path, weak_handle, m_device }, std::move(config));
            if (!result) {
                log::error("Asset loading failed.", result.error());
            }
        }
    );

    return make_strong<A>(weak_handle);
}

} // namespace siren
