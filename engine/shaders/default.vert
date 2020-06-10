#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 normal;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;


layout(set=1, binding = 0) uniform UniformBufferObject {
    mat4 trans;
    mat4 modelTrans;
} model;

void main() {
    gl_Position = model.trans * inPosition;
    vec4 norm = vec4(inNormal.xyx, 0);
    normal = model.modelTrans * norm;
}