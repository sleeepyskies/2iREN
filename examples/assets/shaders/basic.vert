#version 460
layout(location = 0) in vec3 a_pos;
layout(location = 0) out vec3 v_pos;

layout(binding = 0) uniform UBO {
    mat4 rot;
};

void main() {
    gl_Position = rot * vec4(a_pos, 1.0);
    v_pos = a_pos;
}
