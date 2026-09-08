#pragma once

#include <initializer_list>
#include <type_traits>
#include <vector>

#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"

namespace siren {

/// @brief A container used for creating blobs of binary data.
/// Data can be uploaded and interpreted using templated types.
/// However, after upload it is up to the caller to make sure
/// the data is interpreted correctly, as the buffer will not
/// remember or check this.
class ByteBuffer;

/// @brief A non owning view into a @ref ByteBuffer.
using ByteBufferView = std::span<const u8>;

class ByteBuffer {
public:
    /// @brief Constructs an empty ByteBuffer.
    ByteBuffer() = default;

    template <typename T>
    static constexpr inline auto make(std::initializer_list<T> items) {
        return ByteBuffer{items};
    }

    /// @brief Constructs a ByteBuffer a span of elements and writes them into the buffer.
    template <typename T>
    explicit ByteBuffer(const std::span<T>& data) {
        for (const auto& item : data) {
            write<T>(item);
        }
    }

    /// @brief Constructs a ByteBuffer from an initializer list.
    /// @tparam T Type of the items to insert into the ByteBuffer.
    /// @param items Items to append to the buffer.
    template <typename T>
    explicit ByteBuffer(const std::initializer_list<T> items) {
        write<T>(items);
    }

    ByteBuffer(const ByteBuffer& other)            = default;
    ByteBuffer(ByteBuffer&& other)                 = default;
    ByteBuffer& operator=(const ByteBuffer& other) = default;
    ByteBuffer& operator=(ByteBuffer&& other)      = default;

    /// @brief Returns buffer size in bytes.
    [[nodiscard]]
    auto size_bytes() const noexcept -> usize {
        return m_data.size();
    }

    /// @brief Returns the buffer size for T.
    template <typename T>
    [[nodiscard]]
    auto size_as() const noexcept -> usize {
        return m_data.size() / sizeof(T);
    }

    /// @brief Returns buffer capacity in bytes.
    [[nodiscard]]
    auto capacity_bytes() const noexcept -> usize {
        return m_data.capacity();
    }

    /// @brief Returns buffer capacity for T.
    template <typename T>
    [[nodiscard]]
    auto capacity_as() const noexcept -> usize {
        return m_data.capacity() / sizeof(T);
    }

    /// @brief Resizes the buffer in bytes.
    auto resize_bytes(const usize size_bytes) -> void {
        m_data.resize(size_bytes);
    }

    /// @brief Resizes the buffer for T.
    template <typename T>
    auto resize_as(const usize size_t) -> void {
        m_data.resize(size_t* sizeof(T));
    }

    /// @brief Reserves memory in bytes.
    auto reserve_bytes(const usize size) noexcept -> void {
        m_data.reserve(size);
    }

    /// @brief Reserves memory for N elements of type T.
    template <typename T>
    auto reserve_as(const usize size) noexcept -> void {
        reserve_bytes(size * sizeof(T));
    }

    /// @brief Checks if buffer is empty.
    [[nodiscard]]
    auto empty() const noexcept -> bool {
        return m_data.empty();
    }

    /// @brief Clears the buffer contents.
    auto clear() -> void {
        m_data.clear();
    }

    /// @brief Writes the binary representation of the item into the buffer.
    template <typename T>
    auto write(const T& item) -> void {
        const auto* bytes = reinterpret_cast<const u8*>(&item);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(T));
    }

    /// @brief Writes the items into the buffer.
    template <typename T>
    auto write(const std::initializer_list<T> items) -> void {
        for (const auto& item : items) {
            write(item);
        }
    }

    /// @brief Writes the items into the buffer.
    template <typename T>
    auto write(const std::span<T> items) -> void {
        for (const auto& item : items) {
            write(item);
        }
    }

    /// @brief Returns a pointer to the underlying storage.
    template <typename Self>
    [[nodiscard]]
    auto data(this Self&& self) {
        return std::forward<Self>(self).m_data.data();
    }

    /// @brief Interprets buffer as array of type T.
    /// @warning Requires correct size and alignment.
    /// @return Pointer to the first element of the bytes as T.
    template <typename T, typename Self>
    [[nodiscard]]
    auto as(this Self&& self) {
        std::forward<Self>(self).template assert_size<T>();
        std::forward<Self>(self).template assert_alignment<T>();
        using Value =
            std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const T, T>;
        return reinterpret_cast<Value*>(std::forward<Self>(self).data());
    }

    /// @brief Returns a non owning view into this buffer.
    [[nodiscard]]
    auto view() const noexcept -> ByteBufferView {
        return m_data; // should auto convert for us :D
    }

private:
    std::vector<u8> m_data;

    template <typename T>
    auto assert_size() const -> void {
        ASSERT(size_bytes() % sizeof(T) == 0);
    }

    template <typename T>
    auto assert_alignment() const -> void {
        ASSERT(reinterpret_cast<uintptr_t>(data()) % alignof(T) == 0);
    }
};

} // namespace siren
