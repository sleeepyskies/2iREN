#pragma once

#include <libassert/assert.hpp>
#include <vector>

#include "concepts.hpp"
#include "2iren/base.hpp"


namespace siren {
/**
 * @class ByteBuffer
 * @brief Utility class for uploading various data types to a packed binary buffer.
 * Basically a helper class wrapped around a std::vector<u8> helping to avoid dealing
 * with byte alignment problems and difficulties.
 */
class ByteBuffer {
public:
    /**
     * @brief Constructs an empty ByteBuffer.
     */
    ByteBuffer() = default;

    /**
     * @brief Constructs a ByteBuffer from a vector of trivially copyable elements.
     * @tparam T Type of the items to copy into the ByteBuffer. Must be trivially copyable.
     * @param data Input data to be serialized into bytes.
     */
    template <IsCopyable T>
    explicit ByteBuffer(const std::vector<T>& data) {
        for (const auto& item : data) {
            append<T>(item);
        }
    }

    /**
     * @brief Constructs a ByteBuffer from an initializer list.
     * @tparam T Must satisfy IsCopyable.
     * @param items Items to append to the buffer.
     */
    template <IsCopyable T>
    explicit ByteBuffer(const std::initializer_list<T> items) { append<T>(items); }

    ByteBuffer(const ByteBuffer& other)            = default;
    ByteBuffer(ByteBuffer&& other)                 = default;
    ByteBuffer& operator=(const ByteBuffer& other) = default;
    ByteBuffer& operator=(ByteBuffer&& other)      = default;

    /**
     * @brief Returns buffer size in bytes.
     */
    auto size_bytes() const noexcept -> usize { return m_data.size(); }

    /**
     * @brief Returns number of elements of type T stored in the buffer.
     * @warning Assumes buffer is evenly divisible by sizeof(T).
     */
    template <typename T>
    auto size_as() const noexcept -> usize { return size_bytes() / sizeof(T); }

    /**
     * @brief Checks if buffer is empty.
     */
    auto empty() const noexcept -> bool { return m_data.empty(); }

    /**
     * @brief Reserves memory in bytes.
     */
    auto reserve_bytes(const usize size) noexcept -> void { m_data.reserve(size); }

    /**
     * @brief Reserves memory for N elements of type T.
     */
    template <typename T>
    auto reserve_as(const usize size) noexcept -> void { reserve_bytes(size * sizeof(T)); }

    /**
     * @brief Clears the buffer contents.
     */
    auto clear() -> void { m_data.clear(); }

    /**
     * @brief Appends an object as raw bytes.
     * @tparam T The type of the item to serialize, must be trivially copyable.
     * @param item The item to serialize.
     */
    template <IsCopyable T>
    auto append(const T& item) -> void {
        const auto* bytes = reinterpret_cast<const u8*>(&item);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(T));
    }

    /**
     * @brief Appends an object as raw bytes.
     * @tparam T The type of the item to serialize, must be trivially copyable.
     * @param item The item to serialize.
     * @param align_as To what byte size to align the item as. Must be >= sizeof(T).
     */
    template <IsCopyable T>
    auto append(const T& item, const usize align_as) -> void {
        ASSERT(align_as >= sizeof(T));
        const auto* bytes = reinterpret_cast<const u8*>(&item);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(T));
        const auto padding = align_as - sizeof(T);
        m_data.resize(m_data.size() + padding, u8{0});
    }

    /**
     * @brief Appends multiple objects to the buffer.
     * @tparam T The type of the items to serialize, must be trivially copyable.
     * @param items The items to serialize.
     */
    template <IsCopyable T>
    auto append(const std::initializer_list<T> items) -> void {
        for (const auto& item : items) {
            append(item);
        }
    }

    /**
     * @brief Returns mutable reference to underlying byte storage.
     */
    auto data() -> std::vector<u8>& { return m_data; }

    /**
     * @brief Returns const reference to underlying byte storage.
     */
    auto data() const -> const std::vector<u8>& { return m_data; }


    /**
     * @brief Returns raw pointer to byte data.
     */
    auto raw() -> u8* { return m_data.data(); }

    /**
     * @brief Returns const raw pointer to byte data.
     */
    auto raw() const -> const u8* { return m_data.data(); }

    /**
     * @brief Interprets buffer as array of type T.
     * @warning Requires correct size and alignment.
     * @return Pointer to the first element of the bytes as T.
     */
    template <typename T>
    auto as() -> T* {
        assert_size<T>();
        assert_alignment<T>();
        return reinterpret_cast<T*>(raw());
    }

    /**
     * @brief Interprets buffer as array of type T.
     * @warning Requires correct size and alignment.
     * @return Const pointer to the first element of the bytes as T.
     */
    template <typename T>
    auto as() const -> const T* {
        assert_size<T>();
        assert_alignment<T>();
        return reinterpret_cast<T*>(raw());
    }

private:
    std::vector<u8> m_data{};

    /**
     * @brief Ensures buffer size is a multiple of sizeof(T).
     */
    template <typename T>
    auto assert_size() -> void { ASSERT(size_bytes() % sizeof(T) == 0); }

    /**
     * @brief Ensures buffer is aligned for type T.
     */
    template <typename T>
    auto assert_alignment() -> void { ASSERT(reinterpret_cast<uintptr_t>(raw()) % alignof(T) == 0); }
};
} // namespace siren
