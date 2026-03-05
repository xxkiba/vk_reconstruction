#version 450

#extension GL_KHR_vulkan_glsl : enable

const float PI = 3.14159265359;
layout(set = 0, binding = 2) uniform samplerCube skyboxSampler;
layout(set = 1, binding = 0) uniform sampler2D texSampler[1];


layout(location = 0) in vec3 V_Texcoord;

layout(location = 0) out vec4 FragColor;

vec2 Vec3Texcoord2UV(vec3 inTexcoord){ // Convert spherical coordinates to UV mapping
	float arcsiny = asin(inTexcoord.y); // [-PI/2, PI/2]
	arcsiny/= PI; // Normalize to [-0.5, 0.5]
	arcsiny += 0.5; // Shift to [0, 1]

	float fracZX = inTexcoord.z / inTexcoord.x; // can not use atan(fracZX)directly, the range is [-PI/2, PI/2] which is not suitable for UV mapping
	float atanZX = atan(inTexcoord.z,inTexcoord.x); // [-PI, PI]
	atanZX /= PI; // Normalize to [-1, 1]
	atanZX *=0.5;
	atanZX += 0.5; // Shift to [0, 1]
	return vec2(atanZX, arcsiny); // Return UV coordinates
}


void main() {
	vec3 texcoord = normalize(V_Texcoord);
	vec2 uv = Vec3Texcoord2UV(texcoord);
	vec3 hdriColor = texture(texSampler[0], uv).rgb; // HDR Color
	FragColor = vec4(hdriColor, 1.0); // Output color with alpha set to 1.0
}