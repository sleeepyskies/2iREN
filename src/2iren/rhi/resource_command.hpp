#pragma once

#include <libassert/assert.hpp>

#include "resources/buffer.hpp"
#include "resources/image.hpp"
#include "../util/byte_buffer.hpp"


namespace siren {

/**
 * @brief Identifies the type of operation recorded into the buffer.
 * Acts as a tag for a union.
 */
enum class ResourceCommandType : u8 {
    UploadBuffer,
    UploadImage,
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
};

/**
 * @brief Encapsulates a single resource command. Is essentially a tagged union.
 */
struct ResourceCommand {
    union {
        UploadBuffer upload_buffer_command;
        UploadImage upload_image_command;
    } command;

    ResourceCommandType type;

    /** @brief Attempts to cast the internal command into a Command type. Crashes on fail. */
    template <typename Command>
    auto as() const -> Command {
        if constexpr (std::is_same_v<Command, UploadBuffer>) {
            return command.upload_buffer_command;
        } else if constexpr (std::is_same_v<Command, UploadImage>) {
            return command.upload_image_command;
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

    /** @brief Uploads the given data to an @ref Image. */
    auto upload_to_image(ImageHandle image_handle, std::span<const u8> data) -> void;

    /** @brief Consumes the internal data of the ResourceCommandBuffer ready for execution. */
    [[nodiscard]] auto finish() noexcept -> ResourceCommandBuffer;

private:
    friend class OpenGLCommandExecutor;

    std::vector<ResourceCommand> m_commands;
    std::vector<u8> m_blob;
};

} // namespace siren
