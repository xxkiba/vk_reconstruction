#pragma once
#include "vk_common.h"
#include "device.h"
#include "description.h"
#include "descriptorSetLayout.h"
#include "descriptorPool.h"

namespace VKFW::vulkancore {
	/*
	* For each model, we need to create a descriptor set to bind the uniform buffer to the pipeline, bind position is in command buffer
	* For each DescriptorSet, it contains vpMatrix, modelMatrix, and other uniform buffers, and also the texture, binding size.
	*
	* Because of the existance of swapchain, we need to create a descriptor set for each frame.
	*/
	class DescriptorSet {
	public:
		using Ptr = std::shared_ptr<DescriptorSet>;
		static Ptr create(const VKFW::Ref<Device>& device, const std::vector<VKFW::Ref<UniformParameter>> params, const DescriptorSetLayout::Ptr& layout, const DescriptorPool::Ptr& pool, int frameCount) {
			return std::make_shared<DescriptorSet>(device, params, layout, pool, frameCount);
		}

		DescriptorSet(const VKFW::Ref<Device>& device, const std::vector<VKFW::Ref<UniformParameter>> params, const DescriptorSetLayout::Ptr& layout, const DescriptorPool::Ptr& pool, int frameCount);
		~DescriptorSet();
		[[nodiscard]] auto getDescriptorSet(int frameCount) const {
			return mDescriptorSets[frameCount];
		}
	private:
		std::vector<VkDescriptorSet> mDescriptorSets{};
		VKFW::Ref<Device> mDevice{ nullptr };
	};
}