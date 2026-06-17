#version 460
layout(location = 0) in vec3 v_pos;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(v_pos + 0.5, 1.0);
}
