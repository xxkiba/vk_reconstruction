#pragma once

#include "../ptr.h"
#include "device.h"
#include "commandBuffer.h"
#include "commandPool.h"

namespace VKFW::vulkancore {
	/*
	* Sampler is used to sample the texture, it contains the filter, address mode, and other parameters
	*/
	class Sampler {
	public:
		Sampler(const VKFW::Ref<VKFW::vulkancore::Device>& device, bool isCubeMap = false, bool isRepeat = false);
		~Sampler();

		void createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool isCubeMap = false);
		[[nodiscard]] VkSampler getSampler() const { return mSampler; }
	private:
		VkSampler mSampler{ VK_NULL_HANDLE };
		VKFW::Ref<VKFW::vulkancore::Device> mDevice{ nullptr };
	};
}