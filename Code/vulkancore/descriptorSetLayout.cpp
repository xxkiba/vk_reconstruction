#include "descriptorSetLayout.h"

namespace VKFW::vulkancore {
	DescriptorSetLayout::DescriptorSetLayout(const VKFW::Ref<Device>& device)
		: mDevice(device) {
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		if (mLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(mDevice->getDevice(), mLayout, nullptr);
			mLayout = VK_NULL_HANDLE;
		}
	}

	void DescriptorSetLayout::build(const std::vector<VKFW::Ref<UniformParameter>>& params) {
		mBindingParameters = params;
		if (mLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(mDevice->getDevice(), mLayout, nullptr);
			mLayout = VK_NULL_HANDLE;
		}

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (const auto& param : mBindingParameters) {
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = param->mBinding;
			binding.descriptorType = param->mDescriptorType;
			binding.descriptorCount = param->mCount;
			binding.stageFlags = param->mStageFlags;
			binding.pImmutableSamplers = nullptr; // Only needed for sampler types
			bindings.push_back(binding);
		}
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		if (vkCreateDescriptorSetLayout(mDevice->getDevice(), &layoutInfo, nullptr, &mLayout) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to create descriptor set layout!");
		}
	}
}