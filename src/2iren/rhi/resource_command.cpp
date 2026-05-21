#include "resource_command.hpp"

#include "2iren/util/byte_buffer.hpp"


namespace siren {

auto ResourceCommandRecorder::upload_to_buffer(
    const BufferHandle buffer_handle,
    const ByteBuffer& data,
    const u32 dest_offset
) -> void {
    const UploadBuffer upload{
        .buffer_handle = buffer_handle,
        .blob_offset = m_blob.size(),
        .dest_offset = dest_offset,
        .data_size = data.size_bytes(),
    };

    const auto raw = data.raw();
    m_blob.insert(m_blob.end(), raw, raw + data.size_bytes());

    m_commands.emplace_back(
        ResourceCommand{
            .command = {
                .upload_buffer_command = upload
            },
            .type = ResourceCommandType::UploadBuffer,
        }
    );
}

auto ResourceCommandRecorder::upload_to_image(const ImageHandle image_handle, std::span<const u8> data) -> void {
    const UploadImage upload{
        .image_handle = image_handle,
        .data_offset = m_blob.size(),
        .data_size = data.size(),
    };

    m_blob.insert(m_blob.end(), data.begin(), data.end());

    m_commands.emplace_back(
        ResourceCommand{
            .command = {
                .upload_image_command = upload
            },
            .type = ResourceCommandType::UploadImage,
        }
    );
}

auto ResourceCommandRecorder::finish() noexcept -> ResourceCommandBuffer {
    return ResourceCommandBuffer{ .commands = std::move(m_commands), .blob = std::move(m_blob) };
}

} // namespace siren
