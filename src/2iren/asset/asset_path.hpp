#pragma once

#include <string_view>
#include <optional>
#include <memory>

#include "2iren/util/hashed_string.hpp"


namespace siren {

/**
 * @brief A lightweight shared representation of an assets location in a VFS.
 *
 * AssetPath's follow the structure:
 *
 * @c "vfs://relative/asset/path/file.sr#some_label"
 *
 * Labels are not required. The presence of a label indicates the asset
 * this path refers to is embedded into another assets file.
 */
class AssetPath {
public:
    /** @brief Constructs an invalid AssetPath. */
    AssetPath();

    /**
     * @brief
     * @param vfs The virtual file system mount.
     * @param relative_path The path relative to the virtual file system mount.
     * @param label The optional label of the asset.
     */
    AssetPath(
        std::string_view vfs,
        std::string_view relative_path,
        std::string_view label = ""
    );

    /**
     * @brief Parses an AssetPath from its string representation.
     *
     * Examples of accepted input string formats
     *     - @c "vfs://relative/asset/path/file.sr#some_label"
     *     - @c "vfs://relative/asset/path/file.sr"
     *
     * Examples of invalid formats
     *     - @c "relative/asset/path/file.sr#some_label"
     *     - @c "vfs://relative/asset/path/file.sr#label#more_labels#"
     *     - @c "://relative/asset/path/file.sr#label#more_labels#"
     *
     * @param str Input string in AssetPath format
     * @return Parsed AssetPath or invalid() on failure
     */
    [[nodiscard]] auto static parse(const std::string& str) noexcept -> AssetPath;
    /** @brief Returns an invalid AssetPath. */
    [[nodiscard]] auto static invalid() noexcept -> AssetPath;

    /** @brief Checks if the AssetPath is valid or not. */
    [[nodiscard]] auto is_valid() const noexcept -> bool;

    /** @brief Returns the full VFS path. */
    [[nodiscard]] auto full_path() const noexcept -> std::string_view;

    /** @brief Returns the VFS mount. */
    [[nodiscard]] auto vfs() const -> std::string;
    /** @brief Returns the relative path. */
    [[nodiscard]] auto relative_path() const -> std::string;
    /** @brief Returns the label, if present. */
    [[nodiscard]] auto label() const -> std::optional<std::string>;

    /** @brief Returns the filename of this AssetPath. */
    [[nodiscard]] auto filename() const -> std::string;
    /** @brief Returns the file extension of this AssetPath. */
    [[nodiscard]] auto extension() const -> std::string;
    /** @brief Returns the full string representation of this AssetPath. */
    [[nodiscard]] auto to_string() const noexcept -> std::string_view;

    /** @brief Checks if the AssetPath has a label. */
    [[nodiscard]] auto has_label() const noexcept -> bool;
    /** @brief Returns the @ref HashedString of this AssetPath. */
    [[nodiscard]] auto hashed_string() const noexcept -> HashedString;

private:
    /**
     * @brief The underlying string buffer.
     * @details We use a single shared string buffer here to
     * reduce memory overhead, since WeakHandle and StrongHandle
     * have an AssetPath and are copied and passed around alot.
     */
    std::shared_ptr<const std::string> m_buffer;

    /**
     * @brief The offset of the label into the main buffer.
     * Is not required, in which case it is set to 0.
     * @code
     * "ass://path/to/file.sr#label"
     *                        ^
     * m_label_offset == 22
     * @endcode
     */
    u16 m_label_offset;

    /**
     * @brief The offset of the path into the main buffer.
     * @code
     * "ass://path/to/file.sr#label"
     *        ^
     * m_path_offset == 6
     * @endcode
     */
    u16 m_path_offset;
};

} // namespace siren
