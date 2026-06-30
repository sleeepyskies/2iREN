#version 460
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_normal;
layout(location = 2) in vec4 v_color;
layout(location = 3) in vec2 v_texture;
layout(location = 4) in vec4 v_tangent;

layout(binding = 0) uniform sampler2D u_texture;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = texture(u_texture, v_texture);
}
