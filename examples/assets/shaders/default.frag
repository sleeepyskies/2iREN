#version 460
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_normal;
layout(location = 2) in vec4 v_color;
layout(location = 3) in vec2 v_texture;
layout(location = 4) in vec4 v_tangent;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(1.f, 0.f, 0.f, 1.f);
}
