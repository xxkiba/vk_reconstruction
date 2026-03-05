#version 420
#extension GL_KHR_vulkan_glsl : enable
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;
layout(location=3)in vec4 tangent;

layout(push_constant)uniform PushConstants {
    vec4 offsets[3];
}Constants;

layout(set = 0,binding = 0) uniform NVPMatrices {
    mat4 normalMatrix;
    mat4 view;
    mat4 projection;
}vpUBO;
layout(set = 0,binding = 1) uniform ModelMatrix {
    mat4 model;
}objectUBO;

layout(location=0)out vec4 V_Texcoord;
layout(location=1)out vec4 V_NormalWS;
layout(location=2)out vec4 V_PositionWS;
layout(location=3)out mat3 V_TBN;

void main(){
    vec3 n = normalize((vpUBO.normalMatrix * vec4(normal.xyz, 0.0)).xyz);
    V_NormalWS = vec4(n, 0.0);
    V_Texcoord=texcoord;
    vec3 t=normalize(vec3(objectUBO.model*vec4(tangent.xyz,0.0)));
    vec3 b=normalize(cross(V_NormalWS.xyz,t));
    V_TBN=mat3(t,b,V_NormalWS.xyz);

    vec4 positionMS = vec4(position.xyz,1.0);
    V_PositionWS=objectUBO.model*positionMS;//world space
    gl_Position=vpUBO.projection * vpUBO.view * V_PositionWS;//ndc
}