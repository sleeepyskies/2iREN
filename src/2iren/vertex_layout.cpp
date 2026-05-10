#include "vertex_layout.hpp"


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

constexpr auto DataType::to_string() const -> std::string {
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

auto VertexLayoutBuilder::start() -> VertexLayoutBuilder { return VertexLayoutBuilder{ }; }

auto VertexLayoutBuilder::finish() -> VertexLayout {
    return VertexLayout{
        .attributes = std::move(m_attributes),
        .stride = m_offset,
    };
}

auto VertexLayoutBuilder::add(
    const Component component,
    const u32 count,
    const DataType type
) -> VertexLayoutBuilder& {
    const VertexAttribute attr{
        .component = component,
        .size = count,
        .type = type,
        .offset = m_offset,
        .location = m_attributes.size(),
    };

    m_attributes.push_back(attr);

    m_offset += type.size() * count;

    return *this;
}

} // namespace siren
