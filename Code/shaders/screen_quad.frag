#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 0) uniform sampler2D inputImage;
layout(set = 1, binding = 0) uniform sampler2D texSampler[1];
void main() {
    vec3 hdrColor = texture(texSampler[0], uv).rgb;
    vec3 mapping = hdrColor/(vec3(1.0) + hdrColor);
    vec3 FinalColor = pow(mapping, vec3(1.0/2.2)); // Gamma correction
    outColor = vec4(FinalColor,1.0);
}
