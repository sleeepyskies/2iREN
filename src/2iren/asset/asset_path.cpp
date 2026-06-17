#include "asset_path.hpp"

#include "2iren/util/filesystem.hpp"


namespace siren {

AssetPath::AssetPath() : m_buffer(nullptr), m_label_offset(0), m_path_offset(0) { }

AssetPath::AssetPath(
    const std::string_view vfs,
    const std::string_view relative_path,
    const std::string_view label
) : m_buffer(nullptr), m_label_offset(0), m_path_offset(0) {
    std::string full;
    full.reserve(vfs.size() + relative_path.size() + label.size() + 4);
    full += vfs;
    full += "://";
    full += relative_path;
    m_path_offset = vfs.size() + 3;
    if (!label.empty()) {
        full += "#";
        full += label;
        m_label_offset = m_path_offset + relative_path.size() + 1;
    }
    m_buffer = std::make_shared<const std::string>(std::move(full));
}

auto AssetPath::parse(const std::string& str) noexcept -> AssetPath {
    const auto colon = str.find("://");
    if (colon == std::string::npos) { return invalid(); }

    const auto pound = str.find('#');
    std::string vfs  = str.substr(0, colon);
    std::string path = str.substr(
        colon + 3,
        pound == std::string::npos ? std::string::npos : pound - (colon + 3)
    );
    std::string label = pound == std::string::npos ? "" : str.substr(pound + 1);
    return AssetPath{ std::move(vfs), std::move(path), std::move(label) };
}

auto AssetPath::invalid() noexcept -> AssetPath { return AssetPath{ }; }

auto AssetPath::is_valid() const noexcept -> bool { return m_buffer != nullptr; }

auto AssetPath::full_path() const noexcept -> std::string_view { return *m_buffer.get(); }

auto AssetPath::vfs() const -> std::string { return m_buffer->substr(0, m_path_offset); }

auto AssetPath::relative_path() const -> std::string {
    return m_buffer->substr(m_path_offset, m_label_offset == 0 ? std::string::npos : m_label_offset - m_path_offset);
}

auto AssetPath::label() const -> std::optional<std::string> {
    if (m_label_offset == 0) { return std::nullopt; }
    return m_buffer->substr(m_label_offset + 1);
}

auto AssetPath::filename() const -> std::string { return Path{ relative_path() }.filename().string(); }

auto AssetPath::extension() const -> std::string { return Path{ relative_path() }.extension().string().substr(1); }

auto AssetPath::to_string() const noexcept -> std::string_view { return full_path(); }

auto AssetPath::has_label() const noexcept -> bool { return m_label_offset != 0; }

auto AssetPath::hashed_string() const noexcept -> HashedString {
    if (!m_buffer) { return HashedString{ }; }
    return HashedString{ m_buffer.get()->data() };
}

} // namespace siren
