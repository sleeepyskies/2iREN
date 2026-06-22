#include "asset_server.hpp"

#include "assets/gltf.hpp"
#include "assets/shader.hpp"


namespace siren {

AssetServer::AssetServer(Device& device) : m_device(device) {
    // register_loader(std::make_unique<TextureLoader>());
    register_loader(std::make_unique<GltfLoader>());
    register_loader(std::make_unique<ShaderLoader>());
}

} // namespace siren
