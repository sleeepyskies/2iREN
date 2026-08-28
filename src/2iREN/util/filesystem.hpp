#pragma once

#include <string>
#include <optional>
#include <vector>
#include <span>
#include <system_error>
#include <fstream>
#include <filesystem>

#include "2iREN/base.hpp"
#include "2iREN/sync/mutex.hpp"

/**
 * @brief Collection of utility methods for interacting with a virtual filesystem.
 *
 */
namespace siren {
/**
 * @brief Determines the access rights present when interacting
 * with a file.
 */
enum class FileOpenMode {
    /** @brief Simple read rights from an existing file. */
    Read,
    /** @brief Overwrite the files data, if it exists. */
    Write,
    /** @brief Append to a given existing file. */
    Append,
    /** @brief Both read and write privileges. */
    ReadWrite
};

/** @brief Simple type alias for std::filesystem::path. */
using Path = std::filesystem::path;

/**
 * @brief File abstraction in siren.
 *
 * @note This is a heavy class due to using std::ifstream.
 */
class File {
public:
    File(const Path& path, FileOpenMode mode);
    ~File();
    File(const File&)                    = delete;
    auto operator=(const File&) -> File& = delete;
    File(File&& other) noexcept;
    auto operator=(File&& other) noexcept -> File&;

    /** @brief Checks if it is possible to read from this file. */
    [[nodiscard]] auto can_read() const noexcept -> bool;
    /** @brief Checks if it is possible to write to this file. */
    [[nodiscard]] auto can_write() const noexcept -> bool;
    /** @brief Returns this files path. */
    [[nodiscard]] auto path() const noexcept -> Path;

    /** @brief Returns this files size. */
    [[nodiscard]] auto size() const noexcept -> std::optional<u32>;

    /** @brief Reads into the given buffer. Returns the number of bytes read into the buffer. */
    auto read(std::span<u8> buffer) -> u32;
    /** @brief Reads the entire file contents and returns it. */
    [[nodiscard]] auto read_all() -> std::optional<std::vector<u8>>;
    /** @brief Reads the entire contents of the file as a string and returns it. */
    [[nodiscard]] auto read_all_text() -> std::optional<std::string>;

    /** @brief Writes from the given buffer to the file. */
    auto write(std::span<const u8> buffer) -> bool;

private:
    FileOpenMode m_mode;
    Path m_path;
    std::optional<u32> m_size;
    Mutex<std::fstream> m_stream;
};

} // namespace siren

namespace siren::FileSystem {

/** @brief Mounts a new virtual file system path. */
auto mount(const std::string& virtual_path, const Path& physical_path) -> void;

/** @brief Unmounts a virtual file system path if present. */
auto unmount(const std::string& v_key) -> void;

/** @brief Returns the physical path of a virtual file system key string. */
auto get_physical_path(std::string_view v_key) -> std::optional<Path>;

/** @brief Takes a physical path, and makes it a virtual path of the given v_key, if it exists. */
auto to_virtual(const Path& path, std::string_view v_key) -> std::optional<Path>;

/** @brief Takes a virtual path, and resolves it to a physical path. */
auto to_physical(const Path& path) -> std::optional<Path>;

/** @brief Checks if the given path is virtual. */
auto is_virtual(const Path& path) -> bool;

/** @brief Checks if the given path is physical. */
auto is_physical(const Path& path) -> bool;

/** @brief Checks if a file/directory exists at the given path. */
auto exists(const Path& path) -> bool;

/** @brief Checks if a file exists at the given path. */
auto is_file(const Path& path) -> bool;

/** @brief Checks if a directory exists at the given path. */
auto is_dir(const Path& path) -> bool;

/** @brief Returns the size of the file at the path, if it exists. */
auto get_file_size(const Path& path) -> std::optional<u64>;

/** @brief Takes either a virtual or physical path, and reads max(buffer.size, file.size) bytes into it. */
auto read_into(const Path& path, std::span<u8> buffer) -> bool;

/** @brief Returns a byte vector of the entire file contents, if present. */
auto read_bytes(const Path& path) -> std::optional<std::vector<u8>>;

/** @brief Returns a string of the entire file contents, if present. */
auto read_text(const Path& path) -> std::optional<std::string>;

/** @brief Writes the given data to the file at the path. */
auto write(const Path& path, const std::span<u8> buf) -> bool;

/** @brief Writes the string to the file at the path. */
auto write(const Path& path, const std::string& str) -> bool;

/** @brief Returns a new File object from the given path and IOMode. */
auto open(const Path& path, FileOpenMode mode) -> std::optional<File>;

} // namespace siren::io::Filesystem
