#pragma once


#include "../util/byte_buffer.hpp"
#include "2iREN/math/color.hpp"
#include "resources/buffer.hpp"
#include "resources/image.hpp"

namespace siren {
/**
 * @brief Identifies the type of operation recorded into the buffer.
 * Acts as a tag for a union.
 */
enum class ResourceCommandType : u8 {
    UploadBuffer,
    UploadImage,
    ClearImage,
};

/**
 * @brief Parameters for CPU to GPU data upload @ref Buffer copy.
 */
struct UploadBuffer {
    /** @brief The handle of the target @ref Buffer. */
    BufferHandle buffer_handle;
    /** @brief The offset of the data to copy in the @ref ResourceCommandBuffer blob. */
    usize blob_offset;
    /** @brief The offset to write into the destination buffer. */
    usize dest_offset;
    /** @brief The size of the data to copy in the @ref ResourceCommandBuffer blob. */
    usize data_size;
};

/**
 * @brief Parameters for CPU to GPU data upload @ref Image copy.
 */
struct UploadImage {
    /** @brief The handle of the target @ref Image. */
    ImageHandle image_handle;
    /** @brief The offset of the data to copy in the @ref ResourceCommandBuffer blob. */
    usize data_offset;
    /** @brief The size of the data to copy in the @ref ResourceCommandBuffer blob. */
    usize data_size;
    /** @brief The layer of the image upload. Only used for cube maps. */
    u32 layer;
};

using ClearImageValue = std::variant<Rgba, u32>;

/**
 * @brief Parameters for clearing an @ref Image.
 */
struct ClearImage {
    /** @brief The handle of the target @ref Image. */
    ImageHandle image_handle;
    /** @brief The value to clear the image with. */
    ClearImageValue value;
};

/**
 * @brief Encapsulates a single resource command. Is essentially a tagged union.
 */
struct ResourceCommand {
    union {
        UploadBuffer upload_buffer;
        UploadImage upload_image;
        ClearImage clear_image;
    } command;

    ResourceCommandType type;

    /** @brief Attempts to cast the internal command into a Command type. Crashes on fail. */
    template <typename Command>
    auto as() const -> Command {
        if constexpr (std::is_same_v<Command, UploadBuffer>) {
            return command.upload_buffer;
        } else if constexpr (std::is_same_v<Command, UploadImage>) {
            return command.upload_image;
        } else if constexpr (std::is_same_v<Command, ClearImage>) {
            return command.clear_image;
        } else {
            static_assert(false, "Invalid Resource Command type");
            PANIC("Invalid Resource Command. Cannot cast correctly");
        }
    }
};

/**
 * @brief Represents the result of recording commands
 * into the @ref ResourceCommandBuffer.
 */
struct ResourceCommandBuffer {
    /** @brief The recorded commands. */
    std::vector<ResourceCommand> commands;
    /** @brief Raw storage for all upload tasks. */
    std::vector<u8> blob;
};

/**
 * @brief Records resource operations such as data uploads/copies.
 * Is not designed to be thread safe, as it is assumed each thread
 * receives a unique ResourceCommandBuffer to use.
 *
 * Should be passed to the @ref Device once completed.
 */
class ResourceCommandRecorder {
public:
    ResourceCommandRecorder()  = default;
    ~ResourceCommandRecorder() = default;

    /**
     * @brief Uploads the given CPU data to a @ref Buffer.
     * The caller must ensure the @ref Buffer is large enough.
     */
    auto upload_to_buffer(BufferHandle buffer_handle, const ByteBuffer& data, u32 dest_offset) -> void;

    /**
     * @brief Uploads the given data to the provided Image.
     * @param image_handle The image to upload to.
     * @param data The data to upload
     * @param layer The layer of the upload in accordance to the main image. Only used for cube maps.
     */
    auto upload_to_image(ImageHandle image_handle, std::span<const u8> data, u32 layer = 0) -> void;

    /**
     * @brief Clears the given @ref Image with the provided color.
     * @param image_handle The image to upload to.
     * @param color The color to clear.
     */
    auto clear_image(ImageHandle image_handle, const Rgba color) -> void;

    /**
     * @brief Clears the given @ref Image with the provided color.
     * @param image_handle The image to upload to.
     * @param value The value to clear.
     */
    auto clear_image(ImageHandle image_handle, const u32 value) -> void;

    /** @brief Consumes the internal data of the ResourceCommandBuffer ready for execution. */
    [[nodiscard]] auto finish() noexcept -> ResourceCommandBuffer;

private:
    friend class OpenGLCommandExecutor;

    std::vector<ResourceCommand> m_commands;
    std::vector<u8> m_blob;
};
} // namespace siren
