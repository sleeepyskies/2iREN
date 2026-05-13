#pragma once

#include <vector>
#include <libassert/assert.hpp>

#include "base.hpp"


namespace siren {

/**
 * @enum Component
 * @brief Identifies the semantic purpose of a shader attribute.
 */
enum class Component : u8 {
    Position,
    Normal,
    Tangent,
    Bitangent,
    Texture,
    Color,
};

/**
 * @struct DataType
 * @brief Simple enum like class representing a data type, whilst providing some extra functionality.
 */
struct DataType {
    enum Value {
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64,
        Float16, Float32, Float64,
    } value;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr DataType(const Value v) : value(v) { }
    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator Value() const { return value; }

    /** @brief Returns the size of this DataType instance in bytes. */
    constexpr auto size() const -> usize;
    /** @brief Returns the string representation of this value. */
    constexpr auto to_string() const -> std::string;
};

/** @brief Represents a single attribute inside a buffer. */
struct Attribute {
    /** @brief The type of this attribute. */
    Component component;
    /** @brief The number of components per vertex attribute */
    u32 size;
    /** @brief The datatype of this vertex attribute */
    DataType type;
    /** @brief The byte offset of the first vertex attribute into the whole buffer. */
    usize offset;
    /** @brief The location this attribute is bound to. */
    usize location;
};

/** @brief Describes the layout of a vertex buffer. */
struct Layout {
    /** @brief The various attributes within this buffer. */
    std::vector<Attribute> attributes;
    /** @brief The total stride of a single vertex inside the buffer. */
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
    static auto start() -> LayoutBuilder;

    /** @brief Finishes the construction and returns a @ref VertexLayout instance. */
    auto finish() -> Layout;

    /**
     * @param component The @ref Component to add.
     * @param count The number of components
     * @param type The datatype of the attributes components.
     * Example:
     * @code
     * add(VertexAttribute::Position, 3, DataType::Float32)
     * @endcode
     * This creates a new element within the layout of a vec3f representing position.
     * @return A reference to the builder.
     */
    auto add(
        Component component,
        u32 count,
        DataType type
    ) -> LayoutBuilder&;

private:
    LayoutBuilder() = default;

    std::vector<Attribute> m_attributes{ };
    usize m_offset{ 0 };
};

} // namespace siren
