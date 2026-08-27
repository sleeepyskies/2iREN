#pragma once

#include <libassert/assert.hpp>
#include <vector>

#include "2iREN/base.hpp"

namespace siren {
/**
 * @enum Attribute
 * @brief Identifies the semantic purpose of a shader attribute.
 */
enum class Attribute : u8 {
    Position,
    Normal,
    Color,
    Tangent,
    Texture,
};

/**
 * @struct DataType
 * @brief Simple enum like class representing a data type, whilst providing some extra functionality.
 */
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

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr DataType(const Value v) : value(v) {}
    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator Value() const { return value; }

    /** @brief Returns the size of this DataType instance in bytes. */
    [[nodiscard]] constexpr auto size() const -> usize;
    /** @brief Returns the string representation of this value. */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;
};

/**
 * @brief Represents a single vertex component inside a buffer.
 */
struct Component {
    /** @brief The datatype of this vertex attribute */
    DataType type;
    /** @brief The number of components per vertex attribute */
    u32 size;
    /** @brief The byte offset of the first vertex attribute into the whole buffer. */
    usize offset;
    /** @brief The location this attribute is bound to. */
    usize location;
    /** @brief The attribute of this component. */
    Attribute attribute;
};

/** @brief Describes the layout of a vertex buffer. */
struct Layout {
    /** @brief The various components within this buffer. */
    std::vector<Component> components;
    /**
     * @brief The total stride of a single vertex inside the buffer.
     * This is also equal to the size of a single vertex.
     */
    usize stride;
};

/**
 * @class LayoutBuilder
 * @brief Utility class for building a @ref VertexLayout.
 */
class LayoutBuilder {
public:
    /**
     * @brief Entry function for creating a @ref VertexLayout.
     * @return A newly created @ref VertexLayoutBuilder.
     */
    [[nodiscard]] static auto create() noexcept -> LayoutBuilder;

    /** @brief Finishes the construction and returns a @ref VertexLayout instance. */
    [[nodiscard]] auto finish() -> Layout;

    /**
     * @param attribute The @ref Attribute to add.
     * @param count The number of components
     * @param type The datatype of the attributes components.
     * Example:
     * @code
     * add(VertexAttribute::Position, 3, DataType::Float32)
     * @endcode
     * This creates a new element within the layout of a vec3f representing position.
     * @return A reference to the builder.
     */
    [[nodiscard]] auto add(Attribute attribute, u32 count, DataType type) -> LayoutBuilder&;

private:
    LayoutBuilder() = default;

    std::vector<Component> m_components{};
    usize m_offset{0};
};

/**
 * @brief The default vertex layout of 2iREN. This is a temp solution, but provides some consistency when writing
 * shaders.
 */
const auto DEFAULT_VERTEX_LAYOUT = LayoutBuilder::create()
                                   .add(Attribute::Position, 4, DataType::Float32)
                                   .add(Attribute::Normal, 4, DataType::Float32)
                                   .add(Attribute::Color, 4, DataType::Float32)
                                   .add(Attribute::Texture, 2, DataType::Float32)
                                   .add(Attribute::Tangent, 4, DataType::Float32)
                                   .finish();

/**
 * @brief A minimal default vertex layout for 2iREN.
 */
const auto MINIMAL_VERTEX_LAYOUT = LayoutBuilder::create()
                                   .add(Attribute::Position, 4, DataType::Float32)
                                   .add(Attribute::Normal, 4, DataType::Float32)
                                   .add(Attribute::Texture, 2, DataType::Float32)
                                   .finish();

/**
 * @brief A simple reusable layout for fullscreen shaders.
 */
const auto FULLSCREEN_VERTEX_LAYOUT = LayoutBuilder::create()
                                      .add(Attribute::Texture, 2, DataType::Float32)
                                      .finish();
} // namespace siren
