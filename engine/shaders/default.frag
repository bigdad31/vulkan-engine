#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 normal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDirection = vec3(-1.0, -1.0, 1.0);
    outColor = vec4(1.0, 1.0, 1.0, 1.0) * clamp(dot(normal.xyx, lightDirection), 0.1, 1.0);
}