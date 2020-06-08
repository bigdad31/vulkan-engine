#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;


layout(set=1, binding = 0) uniform UniformBufferObject {
    mat4 trans;
} model;

void main() {
    gl_Position = model.trans * inPosition;
    fragColor = inColor;
}