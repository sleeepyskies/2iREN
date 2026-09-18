#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float4 color;
};

struct VertexIn {
    float2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

vertex auto vmain(VertexIn in [[stage_in]]) -> VertexOut {
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.color = in.color;
    return out;
}

fragment auto fmain(VertexOut in [[stage_in]]) -> float4 {
    return in.color;
}
