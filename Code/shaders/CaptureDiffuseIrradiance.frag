#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 4) uniform samplerCube hdrSampler;

layout(location = 0) in vec3 V_Texcoord;
layout(location = 0) out vec4 FragColor;
const float PI = 3.14159265359;
void main() {
	// Calculate Local Coordinate axis, in terms of world space
	vec3 Z = normalize(V_Texcoord); // Fornt vector, equal to the normal vector passed by the vertex shader
	vec3 Y = vec3(0.0, 1.0, 0.0); // Up vector
	vec3 X = normalize(cross(Y, Z)); // Right vector
	Y = normalize(cross(Z, X)); // Recalculate Up vector to ensure orthogonality

	vec3 precomputedLight = vec3(0.0);

	// Do integral over the hemisphere
	float phiStep = 2.0 * PI / 1000; // Number of steps in phi direction
	float thetaStep = 0.5 * PI / 250; // Number of steps in theta direction

	float sampleCount = 0.0; // Initialize sample count
	for (float phi = 0.0; phi < 2.0 * PI; phi += phiStep) {
		for (float theta = 0.0; theta < 0.5 * PI; theta += thetaStep) {
			vec3 localL=vec3(sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta));
			vec3 sampleDirection = localL.x * X + localL.y * Y + localL.z * Z; // Convert to world space direction
			vec3 texcoord = normalize(sampleDirection);
			vec3 hdriColor = texture(hdrSampler, texcoord).rgb;
			precomputedLight += hdriColor * cos(theta) * sin(theta);
			sampleCount += 1.0; // Increment sample count
		}
	}

	precomputedLight = PI * precomputedLight * (1.0 / sampleCount); // Average the samples and scale by PI

	FragColor = vec4(precomputedLight, 1.0); // Output color with alpha set to 1.0
}