#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 4) uniform samplerCube hdrSampler;

layout(location = 0) in vec3 V_Texcoord;
layout(location = 0) out vec4 FragColor;

void main() {
	vec3 texcoord = normalize(V_Texcoord);
	vec3 hdriColor = texture(hdrSampler,texcoord).rgb; // HDR Color
	FragColor = vec4(hdriColor, 1.0); // Output color with alpha set to 1.0
}