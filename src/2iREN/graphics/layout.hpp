#pragma once

#include <vector>

#include "2iREN/core/base.hpp"

namespace siren {

/// @brief Simple enum like class representing a data type, whilst providing some
/// extra functionality.
/// TODO: this shouldnt be in this file, should be it's own file or something.
struct DataType {
    enum Value {
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float16,
        Float32,
        Float64,
    } value;

    constexpr DataType(const Value v) : value(v) { }
    constexpr operator Value() const {
        return value;
    }

    /// @brief Returns the size of this DataType instance in bytes.
    [[nodiscard]] constexpr auto size() const -> usize;
    /// @brief Returns the string representation of this value.
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;
};

/// @brief Identifies the semantic purpose of a shader attribute.
enum class Attribute : u8 {
    Position,
    Normal,
    Color,
    Tangent,
    Texture,
};

/// @brief Represents a single vertex component inside a buffer.
struct Component {
    /// @brief The datatype of this vertex attribute.
    DataType type;
    /// @brief The number of components per vertex attribute.
    u32 size;
    /// @brief The byte offset of the first vertex attribute into the whole buffer.
    usize offset;
    /// @brief The location this attribute is bound to.
    usize location;
    /// @brief The attribute of this component.
    Attribute attribute;
};

/// @brief Describes the layout of a vertex buffer.
struct Layout {
    /// @brief The various components within this buffer.
    std::vector<Component> components;
    /// @brief The total stride of a single vertex inside the buffer.
    /// This is also equal to the size of a single vertex.
    usize stride;
};

/// @brief Utility class for building a @ref VertexLayout.
class LayoutBuilder {
public:
    /// @brief Instantiates a new LayoutBuilder.
    [[nodiscard]]
    static constexpr auto make() noexcept -> LayoutBuilder {
        return LayoutBuilder{};
    }

    /// @brief Finishes the construction and returns a @ref VertexLayout instance.
    [[nodiscard]]
    auto finish() -> Layout;

    /// @brief Adds a new component to the vertex layout.
    /// @param attribute The @ref Attribute to add.
    /// @param count The number of components
    /// @param type The datatype of the attributes components.
    /// @return A reference to the builder.
    [[nodiscard]]
    auto add(Attribute attribute, u32 count, DataType type) -> LayoutBuilder&;

private:
    std::vector<Component> m_components{};
    usize m_offset{0};
};

/// @brief The default vertex layout of 2iREN. This is a temp solution, but
/// provides some consistency when writing shaders.
const auto DEFAULT_VERTEX_LAYOUT = LayoutBuilder::make()
                                       .add(Attribute::Position, 4, DataType::Float32)
                                       .add(Attribute::Normal, 4, DataType::Float32)
                                       .add(Attribute::Color, 4, DataType::Float32)
                                       .add(Attribute::Texture, 2, DataType::Float32)
                                       .add(Attribute::Tangent, 4, DataType::Float32)
                                       .finish();

/// @brief A simple reusable layout for fullscreen shaders.
const auto FULLSCREEN_VERTEX_LAYOUT =
    LayoutBuilder::make().add(Attribute::Texture, 2, DataType::Float32).finish();

} // namespace siren
