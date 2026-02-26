#include "sampler.h"

namespace VKFW::vulkancore {
	Sampler::Sampler(const VKFW::Ref<VKFW::vulkancore::Device>& device, bool isCubeMap, bool isRepeat)
		: mDevice(device) {
		if (isRepeat)
			createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, isCubeMap);
		else
			createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, isCubeMap);
	}
	Sampler::~Sampler() {
		if (mSampler != VK_NULL_HANDLE) {
			vkDestroySampler(mDevice->getDevice(), mSampler, nullptr);
			mSampler = VK_NULL_HANDLE;
		}
	}
	void Sampler::createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool isCubeMap) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = isCubeMap ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : addressMode;
		samplerInfo.addressModeV = isCubeMap ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : addressMode;
		samplerInfo.addressModeW = isCubeMap ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : addressMode;


		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;


		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.flags = isCubeMap ? VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT : 0;//VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT

		// Whether to use unnormalized coordinates
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		// Compare sampling value to the reference value, if true, we need to set the compareOp
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;


		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(mDevice->getDevice(), &samplerInfo, nullptr, &mSampler) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create sampler!");
		}
	}
}