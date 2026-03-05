#version 450
#extension GL_KHR_vulkan_glsl : enable


layout(location=0)in vec3 position;
layout(location=1)in vec3 texcoord;
layout(location=2)in vec3 normal;


layout(set = 0,binding = 0) uniform NVPMatrices {
    mat4 normalMatrix;
    mat4 view;
    mat4 projection;
}vpUBO;
layout(set = 0,binding = 1) uniform ModelMatrix {
    mat4 model;
}objectUBO;

layout(location=0)out vec3 V_Texcoord;

void main(){
    V_Texcoord = position.xyz;
    gl_Position = vpUBO.projection * vpUBO.view * objectUBO.model * vec4(position.xyz, 1.0);
}