#pragma once

#include <unordered_map>
#include <unordered_set>

#include "asset.hpp"
#include "asset_handle.hpp"
#include "asset_id.hpp"
#include "asset_pool.hpp"
#include "asset_loader.hpp"
#include "2iren/sync/rw_lock.hpp"
#include "2iren/sync/thread_pool.hpp"


namespace siren::asset {
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
        /** @brief All active loaders. */
        Loaders loaders;
    };

public:
    /**
     * @brief Attempts to find and return the asset that the provided handle points to.
     * If the asset cannot be found or is still being loaded, this will return a nullptr.
     * @tparam A The specific asset type.
     * @param handle A handle to the asset.
     * @return The asset if it can be found on the server, otherwise a nullptr.
     */
    template <IsAsset A>
    [[nodiscard]] auto get(const StrongHandle<A> handle) -> A* {
        if (!handle.is_valid() || !is_loaded_with_dependencies(handle)) {
            return nullptr;
        }

        return m_data.storage.run(
            [&handle] (const std::unordered_map<TypeID, std::unique_ptr<AssetPoolBase>>& storage){
                const auto it = storage.find(AssetID::get_type_id<A>());
                if (it == storage.end()) { return nullptr; }
                auto* pool = static_cast<AssetPool<A>*>(it->second.get());
                return pool->fetch(handle.id());
            }
        );
    }

    /**
     * @brief Loads an asset from disk, as well as recursively loading all of its dependencies.
     * @tparam A The asset type of the to be loaded asset.
     * @param path The @ref AssetPath of the asset to load.
     * @param config A configuration object with influences how to load the asset.
     * @return A handle to the loaded asset.
     */
    template <IsAsset A>
    [[nodiscard]]
    auto load(const AssetPath& path, AssetLoader<A>::Config* config = nullptr) -> StrongHandle<A>;

    /** @brief Directly adds the provided asset into storage, if a pool exists for its type. */
    template <IsAsset A>
    [[nodiscard]]
    auto add(std::unique_ptr<A>&& asset) -> StrongHandle<A> {
        auto storage = m_data.storage.write();

        const TypeID type_id = AssetID::get_type_id<A>();
        const auto it        = storage->find(type_id);
        if (it == storage->end()) {
            log::error("Could not find an appropriate asset pool for type {}", TypeName<A>());
            return StrongHandle<A>::invalid();
        }

        auto pool        = static_cast<AssetPool<A>*>(it->second.get());
        const AssetID id = pool->add(std::forward<std::unique_ptr<A>>(asset));
        // todo: maybe we can handle non disk assets better, use a UUID or something over an AssetPath?
        return StrongHandle<A>{ id, pool, AssetPath::invalid() };
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
                const auto it = infos.find(handle.path().hashed_string());
                if (it == infos.end()) { return false; }
                return it.second.load_state.get_main() == LoadStatus::Loaded;
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
        return m_data.asset_infos.run(
            [&handle] (const std::unordered_map<HashedString, AssetInfo>& asset_info){
                const auto it = asset_info.find(handle.path().hashed_string());
                if (it == asset_info.end()) { return false; }
                return it.second.load_state.is_ready();
            }
        );
    }

    /**
     * @brief Registers an asset type with the server. No assets of this
     * type can be loaded or stored until registered.
     */
    template <IsAsset A>
    auto register_asset() -> void {
        m_data.storage.run_exclusive(
            [] (std::unordered_map<TypeID, std::unique_ptr<AssetPoolBase>>& storage){
                const auto tid = AssetID::get_type_id<A>();
                if (storage.contains(tid)) { return; }
                storage.emplace(std::make_unique<AssetPool<A>>());
            }
        );
    }

    /**
     * @brief Registers a loader with the asset server. No assets can be
     * loaded from disk until an appropriate loader has been registered.
     * @param loader
     */
    auto register_loader(std::unique_ptr<AssetLoaderBase> loader) -> void {
        auto& loaders         = m_data.loaders;
        const auto loader_ptr = loaders.loaders.emplace_back(std::move(loader)).get();
        for (const auto& ext : loader_ptr->extensions()) {
            loaders.ext_to_loader.insert({ std::string(ext), loader_ptr });
        }
    }

private:
    friend class LoadContext; // Need access to create the dependency tree.

    AssetServerData m_data; ///< @brief The underlying data of the AssetServer.
};

/**
 * @brief LoadContext serves as the AssetServer API for AssetLoaders. It handles registering sub assets, as well as
 * tracking asset dependencies.
 */
class LoadContext {
public:
    LoadContext(AssetServer& server, const AssetPath& path, const WeakHandle& handle)
        : m_server(server), m_handle(handle), m_path(path) { }

    template <IsAsset A>
    [[nodiscard]]
    auto add_labeled_asset(const std::string& label, std::unique_ptr<A>&& asset) -> StrongHandle<A> {
        const StrongHandle<A> handle = m_server.add<A>(std::move(asset));

        // the asset_info should exist already, if it doesn't please crash, something went wrong :D
        auto asset_infos = m_server.m_data.asset_infos.write();
        auto& asset_info = asset_infos->at(path().hashed_string());

        // some issue with adding the new labeled asset
        if (!handle.is_valid()) {
            asset_info.load_state.set_main(LoadStatus::Fail);
            poison_dependents(m_handle, *asset_infos);
            return handle;
        }

        asset_info.labeled_deps.emplace(label, handle.as_weak());
        return handle;
    }

    template <IsAsset A>
    [[nodiscard]]
    auto load_external_asset(
        const AssetPath& asset_path,
        const AssetLoader<A>::ConfigType* config = nullptr
    ) -> StrongHandle<A> {
        const StrongHandle<A> handle = m_server.load<A>(asset_path, config);

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

        main_asset_info.dependencies.emplace(handle.as_weak());
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

        auto& pool = *dynamic_cast<AssetPool<A>*>(storage->at(AssetID::get_type_id<A>()).get());
        pool.link(m_handle.id(), std::move(asset));
    }

    [[nodiscard]]
    constexpr auto path() const noexcept -> AssetPath { return m_handle.path(); }

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
};

template <IsAsset A>
[[nodiscard]] auto AssetServer::load(
    const AssetPath& path,
    typename AssetLoader<A>::Config* config
) -> StrongHandle<A> {
    auto storage     = m_data.storage.write();
    auto asset_infos = m_data.asset_infos.write();

    const auto storage_it = storage->find(AssetID::get_type_id<A>());
    // cannot store this type
    if (storage_it == storage->end()) { return StrongHandle<A>::invalid(); }
    auto pool = static_cast<AssetPool<A>*>(storage_it->second.get());

    // asset has already been cached
    const auto asset_info_it = asset_infos->find(path.hashed_string());
    if (asset_info_it != asset_infos->end()) {
        // this is a labeled sub asset
        const auto label = path.label();
        if (label.has_value()) {
            const auto label_it = asset_info_it->second.labeled_deps.find(label.value());
            if (label_it != asset_info_it->second.labeled_deps.end()) {
                return StrongHandle<A>::from_weak(label_it->second);
            }
        } else {
            // this is the main asset
            return StrongHandle<A>::from_weak(asset_info_it->second.weak_handle);
        }
    }

    // check if loader exists
    const auto loader_it = m_data.loaders.ext_to_loader.find(path.extension());
    if (loader_it == m_data.loaders.ext_to_loader.end()) { return StrongHandle<A>::invalid(); }
    auto loader = dynamic_cast<AssetLoader<A>>(loader_it->second);

    // spawn non-blocking loading task
    AssetID asset_id = pool->reserve();
    const WeakHandle weak_handle{ asset_id, pool, path };
    ThreadPool::get().spawn(
        [this, path, loader, weak_handle, config]{ loader->load(LoadContext{ *this, path, weak_handle }, *config); }
    );

    // init AssetInfo state
    asset_infos->emplace(
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

    return StrongHandle<A>::from_weak(weak_handle);
}

} // namespace siren::asset
