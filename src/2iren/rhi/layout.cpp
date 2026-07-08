#include "layout.hpp"

namespace siren {

/// @todo: do we need to account for padding here?

constexpr auto DataType::size() const -> usize {
    switch (this->value) {
        case Int8:
        case UInt8: return 1;

        case Int16:
        case UInt16:
        case Float16: return 2;

        case Int32:
        case UInt32:
        case Float32: return 4;

        case Int64:
        case UInt64:
        case Float64: return 8;

        default: UNREACHABLE();
    }
}

constexpr auto DataType::to_string() const -> std::string_view {
    switch (this->value) {
        case Int8: return "Int8";
        case Int16: return "Int16";
        case Int32: return "Int32";
        case Int64: return "Int64";
        case UInt8: return "UInt8";
        case UInt16: return "UInt16";
        case UInt32: return "UInt32";
        case UInt64: return "UInt64";
        case Float16: return "Float16";
        case Float32: return "Float32";
        case Float64: return "Float64";
        default: UNREACHABLE();
    }
}

auto LayoutBuilder::start() noexcept -> LayoutBuilder { return LayoutBuilder{}; }

auto LayoutBuilder::finish() -> Layout {
    return Layout{
        .components = std::move(m_components),
        .stride     = m_offset,
    };
}

auto LayoutBuilder::add(const Attribute attribute, const u32 count, const DataType type) -> LayoutBuilder& {
    const Component component{
        .type      = type,
        .size      = count,
        .offset    = m_offset,
        .location  = m_components.size(),
        .attribute = attribute,
    };

    m_components.push_back(component);

    m_offset += type.size() * count;

    return *this;
}

} // namespace siren
