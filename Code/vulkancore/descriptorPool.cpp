#include "descriptorPool.h"

namespace VKFW::vulkancore {
	DescriptorPool::DescriptorPool(const VKFW::Ref<Device>& device)
		: mDevice(device) {
	}
	DescriptorPool::~DescriptorPool() {
		destroyPool();
	}
	void DescriptorPool::build(const std::vector<VKFW::Ref<UniformParameter>>& params, const int& frameCount) {



		int uniformBufferCount = 0;
		int textureCount = 0;
		for (const auto& param : params) {
			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				uniformBufferCount += param->mCount;
			}
			//TODO: add other types of descriptors
			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				textureCount += param->mCount;
			}
		}


		//TODO: texture number

		if (mIsCreate) {
			destroyPool();
		}

		std::vector<VkDescriptorPoolSize> poolSizes{};

		VkDescriptorPoolSize uniformDescriptorSize{};
		uniformDescriptorSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformDescriptorSize.descriptorCount = uniformBufferCount * frameCount;
		if (uniformDescriptorSize.descriptorCount != 0) {
			poolSizes.push_back(uniformDescriptorSize);
		}


		VkDescriptorPoolSize textureDescriptorSize{};
		textureDescriptorSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureDescriptorSize.descriptorCount = textureCount * frameCount; // how much descriptor we need, should be the same as the number of images in the swapchain
		if (textureDescriptorSize.descriptorCount != 0) {
			poolSizes.push_back(textureDescriptorSize);
		}

		mPoolSizes = poolSizes;



		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
		poolInfo.pPoolSizes = mPoolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(frameCount);

		if (vkCreateDescriptorPool(mDevice->getDevice(), &poolInfo, nullptr, &mPool) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to create descriptor pool!");
		}
		mIsCreate = true;
	}
	// Destroy the descriptor pool
	void DescriptorPool::destroyPool() {
		if (mIsDestroy) {
			return;
		}
		if (mPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mDevice->getDevice(), mPool, nullptr);
			mPool = VK_NULL_HANDLE;
		}
		mIsDestroy = true;
		mIsCreate = false;
		mIsReset = false;
		mIsAllocate = false;
		mIsFree = false;
	}
}