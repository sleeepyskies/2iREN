#include "asset_server.hpp"

#include "2iREN/asset/gltf.hpp"
#include "2iREN/asset/shader.hpp"
#include "2iREN/graphics/device.hpp"

namespace siren {

/*
static auto generate_texture(Device& device, const std::string& name) -> Image {
    auto x = device.create_image({
        .label = name,
        .format = ImageFormat::Color8,
        .extent = ImageExtent{
            .width = 2,
            .height = 2,
            .depth_or_layers = 1,
        },
        .dimension = ImageDimension::D2,
        .mipmap_levels = 1,
    });

    auto cmds = device.record_resource_commands();
    cmds.upload_to_image(x.handle(), { todo });

    return x;
}
*/

AssetServer::AssetServer(Device& device) : m_device(device) {
    log::info("Setting up default AssetServer.");

    // register stock loaders

    register_loader(std::make_unique<GltfLoader>());
    register_loader(std::make_unique<ShaderLoader>());
    register_loader(std::make_unique<TextureLoader>());

    // register default assets
    /*
    register_default<Texture>();
    register_default<PBRMaterialAsset>();
    */
}

} // namespace siren
