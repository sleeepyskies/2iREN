#include "filesystem.hpp"

#include "libassert/assert.hpp"


namespace siren {
// ============================================================================
// == MARK: FILE
// ============================================================================

File::File(const Path& path, const FileOpenMode mode) : m_mode(mode), m_path(path), m_size(std::nullopt) {
    auto open_mode = std::ios::binary | std::ios::ate;

    if (mode == FileOpenMode::Read) open_mode |= std::ios::in;
    else if (mode == FileOpenMode::Write) open_mode |= std::ios::out | std::ios::trunc;
    else if (mode == FileOpenMode::Append) open_mode |= std::ios::out | std::ios::app;
    else if (mode == FileOpenMode::ReadWrite) open_mode |= std::ios::in | std::ios::out;

    auto guard = m_stream.lock();
    guard->open(m_path, open_mode);
    if (!guard->is_open()) {
        return;
    }
    m_size = static_cast<u32>(guard->tellg());
    // cache size
    guard->seekg(std::ios::beg);
}

File::~File() {
    auto guard = m_stream.lock();
    if (guard->is_open()) {
        guard->close();
    }
}

File::File(
    File&& other
) noexcept : m_mode(other.m_mode),
             m_path(std::move(other.m_path)),
             m_size(other.m_size),
             m_stream(Mutex(other.m_stream.consume())) { }

auto File::operator=(File&& other) noexcept -> File& {
    if (this != &other) {
        {
            auto guard = m_stream.lock();
            if (guard->is_open()) { guard->close(); }
        }

        m_mode = other.m_mode;
        m_path = std::move(other.m_path);
        m_size = other.m_size;
        m_stream.set(other.m_stream.consume());

        other.m_size = std::nullopt;
    }

    return *this;
}

auto File::can_read() const noexcept -> bool {
    return (m_mode == FileOpenMode::Read || m_mode == FileOpenMode::ReadWrite) && m_stream.lock()->is_open() && m_size.
            has_value();
}

auto File::can_write() const noexcept -> bool {
    return
            (m_mode == FileOpenMode::Write ||
                m_mode == FileOpenMode::ReadWrite)
            && m_stream.lock()->is_open() && m_size.has_value();
}

auto File::path() const noexcept -> Path { return m_path; }

auto File::size() const noexcept -> std::optional<u32> { return m_size; }

auto File::read(const std::span<u8> buffer) -> u32 {
    if (!can_read()) { return 0; }

    const u32 bufsize = std::min(size().value(), static_cast<u32>(buffer.size_bytes()));

    auto guard = m_stream.lock();
    guard->read(reinterpret_cast<char*>(buffer.data()), bufsize);
    return static_cast<u32>(guard->gcount());
}

auto File::read_all() -> std::optional<std::vector<u8>> {
    if (!can_read()) { return std::nullopt; }
    std::vector<u8> buffer(size().value());
    if (m_size.value() == 0) { return buffer; }

    m_stream.run([] (std::fstream& stream){ stream.seekg(std::ios::beg); });

    if (read(buffer) > 0) { return buffer; }
    return std::nullopt;
}

auto File::read_all_text() -> std::optional<std::string> {
    return read_all().transform(
        [] (const std::vector<u8>& buffer){
            return std::string(buffer.begin(), buffer.end());
        }
    );
}

auto File::write(const std::span<const u8> buffer) -> bool {
    if (!can_write()) { return false; }
    ASSERT(m_size.has_value(), "Cannot write to file with non existent size");
    m_size     = m_size.value() + static_cast<u32>(buffer.size_bytes());
    auto guard = m_stream.lock();
    guard->write(reinterpret_cast<const char*>(buffer.data()), buffer.size_bytes());
    return true;
}
}


// ============================================================================
// == MARK: FILESYSTEM
// ============================================================================

namespace siren::FileSystem {

struct Mount {
    std::string virt;
    Path pyhs;
};

// use std::vector here since the amount of mounts should be tiny
static std::vector<Mount> s_mounts;

void mount(const std::string& virtual_path, const Path& physical_path) {
    if (virtual_path.empty()) { return; }
    if (FileSystem::exists(physical_path)) {
        s_mounts.push_back(Mount{ .virt = virtual_path, .pyhs = physical_path });
    }
}

void unmount(const std::string& v_key) {
    for (auto it = s_mounts.begin(); it != s_mounts.end(); ++it) {
        if (it->virt == v_key) { s_mounts.erase(it); }
    }
}

std::optional<Path> get_physical_path(const std::string_view virtual_path) {
    for (const auto& [virt, pyhs] : s_mounts) {
        if (virt == virtual_path) {
            return pyhs;
        }
    }
    return std::nullopt;
}

std::optional<Path> to_virtual(const Path& path, const std::string_view v_key) {
    if (is_virtual(path)) { return path; }

    return get_physical_path(v_key).transform(
        [&v_key, &path] (const Path& physical_path){
            std::error_code ec;
            return v_key / std::filesystem::relative(path, physical_path, ec);
        }
    );
}

std::optional<Path> to_physical(const Path& path) {
    if (is_physical(path)) { return path; }

    std::string p_str = path.string();

    return [] (const std::string& p) -> std::optional<Mount>{
        for (const auto& m : s_mounts) {
            if (p.starts_with(m.virt)) { return m; }
        }
        return std::nullopt;
    }(p_str).transform(
        [&p_str] (const Mount& m){
            const u32 idx = m.virt.length() + 3; // account for ://
            return m.pyhs / p_str.erase(0, idx);
        }
    );
}

bool is_virtual(const Path& path) {
    for (const std::string p_str = path.string(); const auto& m : s_mounts) {
        if (p_str.starts_with(m.virt)) { return true; }
    }
    return false;
}

bool is_physical(const Path& path) { return path.is_absolute(); }

bool exists(const Path& path) {
    return to_physical(path).transform(
        [] (const Path& p){
            std::error_code ec;
            return std::filesystem::exists(p, ec);
        }
    ).value_or(false);
}

bool is_file(const Path& path) {
    return to_physical(path).transform(
        [] (const Path& p){
            std::error_code ec;
            return std::filesystem::is_regular_file(p, ec);
        }
    ).value_or(false);
}

bool is_dir(const Path& path) {
    return to_physical(path).transform(
        [] (const Path& p){
            std::error_code ec;
            return std::filesystem::is_directory(p, ec);
        }
    ).value_or(false);
}

std::optional<u64> get_file_size(const Path& path) {
    return to_physical(path).transform(
        [] (const Path& p){
            std::error_code ec;
            return std::filesystem::file_size(p, ec);
        }
    );
}

bool read_into(const Path& path, std::span<u8> buffer) {
    return to_physical(path).transform(
        [&buffer] (const auto& p){
            if (!is_file(p)) { return false; }
            File file{ p, FileOpenMode::Read };
            if (!file.can_read()) { return false; }
            (void)file.read(buffer);
            return true;
        }
    ).value_or(false);
}

std::optional<std::vector<u8>> read_bytes(const Path& path) {
    return get_file_size(path).transform(
        [&path] (const u32 size){
            std::vector<u8> bytes(size);
            read_into(path, bytes);
            return bytes;
        }
    );
}

std::optional<std::string> read_text(const Path& path) {
    return get_file_size(path).transform(
        [&path] (const u32 size){
            std::string str;
            str.resize(size);
            read_into(path, std::span{ reinterpret_cast<u8*>(str.data()), size });
            return str;
        }
    );
}

bool write(const Path& path, const std::span<u8> buf) {
    auto file = open(path, FileOpenMode::Write);
    if (!file) { return false; }
    return file->write(buf);
}

bool write(const Path& path, const std::string& str) {
    auto file = open(path, FileOpenMode::Write);
    if (!file) { return false; }
    return file->write(std::span{ reinterpret_cast<const u8*>(str.data()), str.size() });
}

std::optional<File> open(const Path& path, FileOpenMode mode) {
    return to_physical(path).transform(
        [mode] (const auto& p){
            return File{ p, mode };
        }
    );
}

} // namespace siren::FileSystem
