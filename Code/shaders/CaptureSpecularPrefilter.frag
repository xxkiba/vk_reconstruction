#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 4) uniform samplerCube hdrSampler;

layout(location = 0) in vec3 V_Texcoord;

layout(push_constant)uniform PushConstants {
    vec4 offsets[3];
}Constants;


layout(location = 0) out vec4 FragColor;
const float PI = 3.14159265359;
float RadicalInverse_VDC(uint inIndex) {
    inIndex = (inIndex << 16u) | (inIndex >> 16u);
    inIndex = ((inIndex & 0x55555555u) << 1u) | ((inIndex & 0xAAAAAAAAu) >> 1u);
    inIndex = ((inIndex & 0x33333333u) << 2u) | ((inIndex & 0xCCCCCCCCu) >> 2u);
    inIndex = ((inIndex & 0x0F0F0F0Fu) << 4u) | ((inIndex & 0xF0F0F0F0u) >> 4u);
    inIndex = ((inIndex & 0x00FF00FFu) << 8u) | ((inIndex & 0xFF00FF00u) >> 8u);
    return float(inIndex) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 HammersleyPoint(uint inIndex,uint inTotalSampleCount){
    return vec2(float(inIndex)/float(inTotalSampleCount), RadicalInverse_VDC(inIndex));
}

vec3 ImportanceSampleGGX(vec2 inXi,vec3 inN,float inRoughness){
    float r4=pow(inRoughness,4);
    //Xi->Spherical Coordnate
    float phi=2.0*PI*inXi.x;
    float cosTheta=sqrt((1.0-inXi.y)/(1.0+(r4-1.0)*inXi.y));//D
    float sinTheta=sqrt(1.0-cosTheta*cosTheta);
    //Spherical -> Cartision
    vec3 H;
    H.x=sinTheta*cos(phi);
    H.y=sinTheta*sin(phi);
    H.z=cosTheta;
    //align -> N
    vec3 temp=abs(inN.z)<0.999?vec3(0.0,0.0,1.0):vec3(1.0,0.0,0.0);
    vec3 X=normalize(cross(temp,inN));
    vec3 Y=normalize(cross(inN,X));

    vec3 HFinal=X*H.x+Y*H.y+inN*H.z;
    return normalize(HFinal);
}

void main() {
	vec3 texcoord = normalize(V_Texcoord);
    vec3 prefilteredColor = vec3(0.0);
    vec3 N = texcoord; // Normal vector, equal to the texture coordinate passed by the vertex shader
    vec3 V = N; // L
    float roughness = Constants.offsets[0].z; // Roughness value, can be replaced with a varying input
    float weight = 0.0;
    const uint sampleCount = 1024; // Number of samples to take

    for (uint i = 0; i < sampleCount; ++i) {
        vec2 xi = HammersleyPoint(i, sampleCount); // Generate Hammersley point
        vec3 H = ImportanceSampleGGX(xi, N, roughness); // Sample the GGX distribution
        vec3 L = normalize(2.0 * dot(N, H) * H - V); // Calculate light direction

        
        float NdotL = max(dot(N, L), 0.0); // Calculate dot product for visibility
        if(NdotL > 0.0){
            vec3 hdriColor = texture(hdrSampler, L).rgb; // Sample the HDR texture
            prefilteredColor += hdriColor * NdotL; // Accumulate color weighted by visibility
            weight += NdotL; // Accumulate weight
        }
    }
    prefilteredColor /= weight; // Average the accumulated color
	FragColor = vec4(prefilteredColor, 1.0); // Output color with alpha set to 1.0
}