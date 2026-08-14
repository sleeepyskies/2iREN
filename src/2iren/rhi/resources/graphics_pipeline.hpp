#pragma once

#include <optional>
#include <string>

#include "fwd.hpp"
#include "2iren/rhi/layout.hpp"


namespace siren {
/**
 * @brief Represents the drawing mode. Aka how points are interpreted and how lines
 * are drawn between them
 */
enum class PrimitiveTopology {
    /** @brief Draw vertices as points. */
    Points,
    /** @brief Every pair of vertices is treated as a line (1-2, 3-4, etc...). */
    Lines,
    /** @brief Chain draw vertices as lines (1-2-3-4-5...) */
    LineStrip,
    /** @brief Every triple of vertices is treated as a triangle (1-2-3, 4-5-6, etc...) */
    Triangles,
    /** @brief Vertices connected in a ribbon (0-1-2, 0-2-3, etc...) */
    TriangleStrip,
    /** @brief First vertex anchors (0-1-2, 0-2-3, 0-3-4, etc...) */
    TriangleFan,
};

/** @brief Defines how the renderer determines a pixels' transparency. */
enum class AlphaMode {
    /** @brief Surface is fully solid. Depth always written to the z-buffer. */
    Opaque,
    /** @brief Semi-transparent. Colors from behind can show through. */
    Blend,
    /** @brief Surface is either fully transparent or fully opaque based on a threshold. */
    Mask,
};

/** @brief The function that determines if a fragment will pass the depth test. */
enum class DepthFunction {
    /** @brief Always pass. */
    Always,
    /** @brief Never pass. */
    Never,
    /** @brief Pass if new < old. */
    Less,
    /** @brief Pass if new == old. */
    Equal,
    /** @brief Pass if new <= old. */
    LessEqual,
    /** @brief Pass if new > old. */
    Greater,
    /** @brief Pass if new >= old. */
    GreaterEqual,
    /** @brief Pass if new != old. */
    NotEqual,
};

/**
 * @brief Defines the function to apply to two alpha values when blending.
 * Aka for: alpha1 . alpha2
 * This will define the behavior of '.'.
 */
enum class BlendFunction {
    /** @brief Adds together the two alpha values. */
    Add,
    /** @brief Subtracts the second alpha value from the first. */
    Subtract,
    /** @brief Subtracts the first alpha value from the second. */
    ReverseSubtract,
    /** @brief Takes the minimum of both alpha values. */
    Min,
    /** @brief Takes the maximum of both alpha values. */
    Max,
};

/**
 * @brief Defines what weights to multiply with the
 */
enum class BlendFactor {
    /** @brief Multiplies all values with 0. */
    Zero,
    /** @brief Multiplies all values with 1. */
    One,
    /** @brief Multiplies with source alpha. */
    SourceAlpha,
    /** @brief Multiplies with destination alpha. */
    DestinationAlpha,
    /** @brief Multiplies with (1 - source alpha). */
    OneMinusSourceAlpha,
    /** @brief Multiplies with (1 - destination alpha). */
    OneMinusDestinationAlpha,
};

/**
 * @brief Collection of parameters describing how to blend together values.
 */
struct BlendDescription {
    /** @brief Describes what function to use to blend 2 values together. */
    BlendFunction function = BlendFunction::Add;
    /** @brief The @ref BlendFactor to affect the source. */
    BlendFactor source_factor = BlendFactor::SourceAlpha;
    /** @brief The @ref BlendFactor to affect the destination. */
    BlendFactor dest_factor = BlendFactor::OneMinusSourceAlpha;
};

struct GraphicsPipelineDescriptor {
    /** @brief An optional label for the @ref GraphicsPipeline. Mainly used for debugging. */
    std::optional<std::string> label = std::nullopt;
    /** @brief How the shader interprets vertex data. */
    Layout layout = DEFAULT_VERTEX_LAYOUT;
    /** @brief The shader to use. */
    ShaderHandle shader;
    /** @brief How to draw vertex data. */
    PrimitiveTopology topology = PrimitiveTopology::Triangles;
    /** @brief Surface transparency type. */
    AlphaMode alpha_mode = AlphaMode::Opaque;
    /** @brief Depth function. */
    DepthFunction depth_function = DepthFunction::Less;
    /*
    union {
        struct {
            /** @brief Describes how to blend color values. Only used if alpha_mode == AlphaMode::Blend.  #
            BlendDescription color_blend;
            /** @brief Describes how to blend alpha values. Only used if alpha_mode == AlphaMode::Blend.  #1#
            BlendDescription alpha_blend;
        };
        BlendDescription color_alpha_blend = {};
    };
    */
    /** @brief Describes how to blend color values. Only used if alpha_mode == AlphaMode::Blend. */
    BlendDescription color_blend;
    /** @brief Describes how to blend alpha values. Only used if alpha_mode == AlphaMode::Blend. */
    BlendDescription alpha_blend;
    /** @brief Whether back face is culled. */
    bool back_face_culling = true;
    /** @brief Whether to perform the depth test. */
    bool depth_test = true;
    /** @brief Whether to write the depth buffer. */
    bool depth_write = true;
};

/**
 * @brief The GraphicsPipeline encapsulates the vertex layout of a buffer, as well
 * as any fixed functions state.
 */
class GraphicsPipeline final : public RenderResource<GraphicsPipeline> {
    using Base = RenderResource<GraphicsPipeline>;

public:
    explicit GraphicsPipeline(Device* device, GraphicsPipelineHandle handle);
    ~GraphicsPipeline();

    GraphicsPipeline(GraphicsPipeline&& other) noexcept;
    GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

    /** @brief Returns the @ref GraphicsPipelineDescriptor used to create this GraphicsPipeline. */
    [[nodiscard]] auto descriptor() const noexcept -> const GraphicsPipelineDescriptor&;
};
} // namespace siren
