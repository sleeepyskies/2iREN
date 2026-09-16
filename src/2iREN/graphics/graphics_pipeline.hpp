#pragma once

#include <optional>
#include <string>

#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/layout.hpp"

namespace siren {

/// @brief Represents the drawing mode. Aka how points are interpreted and how
/// lines are drawn between them
enum class PrimitiveTopology {
    /// @brief Draw vertices as points.
    Points,
    /// @brief Every pair of vertices is treated as a line.
    Lines,
    /// @brief Chain draw vertices as lines.
    LineStrip,
    /// @brief Every triple of vertices is treated as a triangle.
    Triangles,
    /// @brief Vertices connected in a ribbon.
    TriangleStrip,
    /// @brief First vertex anchors.
    TriangleFan,
};

/// @brief Defines how the renderer determines a pixels' transparency.
enum class AlphaMode {
    /// @brief Surface is fully solid. Depth always written to the z-buffer.
    Opaque,
    /// @brief Semi-transparent. Colors from behind can show through.
    Blend,
};

///  @brief The function that determines if a fragment will pass the depth test.
enum class DepthFunction {
    /// @brief Always pass.
    Always,
    /// @brief Never pass.
    Never,
    /// @brief Pass if new < old.
    Less,
    /// @brief Pass if new == old.
    Equal,
    /// @brief Pass if new <= old.
    LessEqual,
    /// @brief Pass if new > old.
    Greater,
    /// @brief Pass if new >= old.
    GreaterEqual,
    /// @brief Pass if new != old.
    NotEqual,
};

/// @brief Defines the function to apply to two alpha values when blending.
/// Aka for: alpha1 . alpha2
/// This will define the behavior of '.'.
enum class BlendFunction {
    /// @brief Adds together the two alpha values.
    Add,
    /// @brief Subtracts the second alpha value from the first.
    Subtract,
    /// @brief Subtracts the first alpha value from the second.
    ReverseSubtract,
    /// @brief Takes the minimum of both alpha values. Note this ignores @ref
    /// BlendFactor.
    Min,
    /// @brief Takes the maximum of both alpha values. Note this ignores @ref
    /// BlendFactor.
    Max,
};

/// @brief Defines what weights to multiply with the
enum class BlendFactor {
    /// @brief Multiplies all values with 0.
    Zero,
    /// @brief Multiplies all values with 1.
    One,
    /// @brief Multiplies with source alpha.
    SourceAlpha,
    /// @brief Multiplies with destination alpha.
    DestinationAlpha,
    /// @brief Multiplies with (1 - source alpha).
    OneMinusSourceAlpha,
    /// @brief Multiplies with (1 - destination alpha).
    OneMinusDestinationAlpha,
};

/// @brief Collection of parameters describing how to blend together values.
struct BlendDescription {
    /// @brief Describes what function to use to blend 2 values together.
    BlendFunction function = BlendFunction::Add;
    /// @brief The @ref BlendFactor to affect the source.
    BlendFactor source_factor = BlendFactor::SourceAlpha;
    /// @brief The @ref BlendFactor to affect the destination.
    BlendFactor dest_factor = BlendFactor::OneMinusSourceAlpha;
};

/// @brief Represents a single color attachment.
struct GraphicsPipelineColorAttachment {
    /// @brief The format of the individual pixels of the image.
    ImageFormat format;
    /// @brief Determines if pixels can be transparent.
    AlphaMode alpha_mode;
    /// @brief Describes how to blend rgb values. Used only when
    /// AlphaMode::Blend.
    BlendDescription color_blend;
    /// @brief Describes how to blend alpha values. Used only when
    /// AlphaMode::Blend.
    BlendDescription alpha_blend;
};

/// @brief Simple alias for a vector of color attachments.
using GraphicsPipelineColorAttachments = std::vector<GraphicsPipelineColorAttachment>;

/// @brief A colletion of parameters used to describe how a @ref
/// GraphicsPipeline should behave.
struct GraphicsPipelineDescriptor {
    /// @brief An optional label.
    Label label = std::nullopt;
    /// @brief The shader to use.
    ShaderHandle shader;
    /// @brief How the vertices are structured. @see LayoutBuilder.
    Layout layout;
    /// @brief The structure of the color attachments this pipeline may access.
    GraphicsPipelineColorAttachments color_attachments;
};

/// @brief A 2iREN API agnostic GraphicsPipeline. Encapsulates render state
/// into a single object.
class GraphicsPipeline final : public RenderResource<GraphicsPipeline> {
    using Base = RenderResource<GraphicsPipeline>;

public:
    /// @brief Creates a new 2iREN GraphicsPipeline.
    /// @warning Should only be called by the @ref Device!
    explicit GraphicsPipeline(Device* device, GraphicsPipelineHandle handle);
    ~GraphicsPipeline();

    GraphicsPipeline(GraphicsPipeline&& other) noexcept;
    GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

    /// @brief Returns the @ref GraphicsPipelineDescriptor of this object.
    [[nodiscard]] auto descriptor() const noexcept -> const GraphicsPipelineDescriptor&;
};
} // namespace siren
