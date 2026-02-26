#include "descriptorSet.h"

namespace VKFW::vulkancore {
	DescriptorSet::DescriptorSet(const VKFW::Ref<Device>& device, const std::vector<VKFW::Ref<UniformParameter>> params, const DescriptorSetLayout::Ptr& layout, const DescriptorPool::Ptr& pool, int frameCount)
		: mDevice(device) {
		mDescriptorSets.resize(frameCount);
		std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->getLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool->getDescriptorPool();
		allocInfo.descriptorSetCount = frameCount;
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(mDevice->getDevice(), &allocInfo, mDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < mDescriptorSets.size(); i++) {
			//for each descriptor set, we need to put params info into the descriptor set
			std::vector<VkWriteDescriptorSet> descriptorWrites;
			std::vector<std::vector<VkDescriptorImageInfo>> imageInfoArrays; // Save All Image Info for Combined Image Sampler

			for (const auto& param : params) {

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mDescriptorSets[i];
				descriptorWrite.dstBinding = param->mBinding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = param->mDescriptorType;
				descriptorWrite.descriptorCount = param->mCount;
				if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					// For combined image sampler, we need to create a vector of VkDescriptorImageInfo
					std::vector<VkDescriptorImageInfo> infos(param->mCount);
					for (size_t j = 0; j < param->mCount; ++j) {
						infos[j] = param->mTextures[i][j]->getImageInfo();
					}
					imageInfoArrays.push_back(std::move(infos));// Need to ensure infos live until the end of vkUpdateDescriptorSets
					descriptorWrite.pImageInfo = imageInfoArrays.back().data();
				}
				else if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					descriptorWrite.pBufferInfo = &param->mBuffers[i]->getBufferInfo();
				}
				descriptorWrites.push_back(descriptorWrite);

			}
			vkUpdateDescriptorSets(mDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
	DescriptorSet::~DescriptorSet() {// Descriptor set will be destroyed by descriptor pool, not need to free it here

	}
}