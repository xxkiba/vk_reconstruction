#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;


layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUV;

layout(set = 0, binding = 0) uniform NVPMatrices {
    mat4 normalMatrix;
    mat4 view;
    mat4 projection;
}vpUBO;
layout(set = 0, binding = 1) uniform ModelMatrix {
    mat4 model;
}objectUBO;
//vec2 position[3] = vec2[](
//    vec2(0.0, -1.0),
//    vec2(0.5, 0.0),
//    vec2(-0.5, 0.0)
//);
//
//vec3 color[3] = vec3[](
//    vec3(1.0, 0.0, 0.0),
//    vec3(0.0, 1.0, 0.0),
//    vec3(0.0, 0.0, 1.0)
//);
//
void main() {
    gl_Position = vpUBO.projection * vpUBO.view * objectUBO.model *vec4(inPosition,1.0);
    outColor = inColor;
    outUV = inUV;
}
