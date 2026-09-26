#pragma once

#include <vector>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/types.hpp"

namespace siren {

/// @brief Represents a unit of data held within a vertex of a vertex buffer.
struct Attribute {
    /// @brief The datatype of this vertex attribute.
    DataType type;
    /// @brief The number of components per vertex attribute.
    u32 size;
    /// @brief The byte offset of the first vertex attribute into the whole buffer.
    usize offset;
    /// @brief The location this attribute is bound to.
    usize location;
};

/// @brief Describes the layout of a vertex buffer.
struct Layout {
    /// @brief The attributes of a single vertex in the buffer.
    std::vector<Attribute> attributes;
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

    /// @brief Adds a new component to the vertex layout.
    /// @param type The datatype of the attributes components.
    /// @param count The number of components
    /// @return A reference to the builder.
    [[nodiscard]]
    auto add(const DataType type, const u32 count) -> LayoutBuilder& {
        m_atttributes.emplace_back(type, count, m_offset, m_atttributes.size());
        m_offset += type.size_bytes() * count;
        return *this;
    }

    /// @brief Finishes the construction and returns a @ref VertexLayout instance.
    [[nodiscard]]
    constexpr auto finish() noexcept -> Layout {
        return Layout{
            .attributes = std::move(m_atttributes),
            .stride     = m_offset,
        };
    }

private:
    std::vector<Attribute> m_atttributes = {};
    usize m_offset                       = 0;
};

/// @brief The default vertex layout of 2iREN. This is a temp solution, but
/// provides some consistency when writing shaders.
const auto DEFAULT_VERTEX_LAYOUT = LayoutBuilder::make()
                                       .add(DataType::Float32, 4)
                                       .add(DataType::Float32, 4)
                                       .add(DataType::Float32, 4)
                                       .add(DataType::Float32, 2)
                                       .add(DataType::Float32, 4)
                                       .finish();

/// @brief A simple reusable layout for fullscreen shaders.
const auto FULLSCREEN_VERTEX_LAYOUT = LayoutBuilder::make().add(DataType::Float32, 2).finish();

} // namespace siren
