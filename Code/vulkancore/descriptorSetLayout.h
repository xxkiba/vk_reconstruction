#pragma once

#include "vk_common.h"
#include "device.h"
#include "description.h"

namespace VKFW::vulkancore {
	class DescriptorSetLayout {
	public:
		using Ptr = std::shared_ptr<DescriptorSetLayout>;
		static Ptr create(const VKFW::Ref<Device>& device) {
			return std::make_shared<DescriptorSetLayout>(device);
		}

		DescriptorSetLayout(const VKFW::Ref<Device>& device);
		~DescriptorSetLayout();

		void build(const std::vector<VKFW::Ref<UniformParameter>>& params);

		[[nodiscard]] auto getLayout() const {
			return mLayout;
		}
		[[nodiscard]] auto getDevice() const {
			return mDevice;
		}
		[[nodiscard]] auto getBindingParameters() const {
			return mBindingParameters;
		}

	private:
		VkDescriptorSetLayout mLayout{ VK_NULL_HANDLE };
		VKFW::Ref<Device> mDevice{ nullptr };
		std::vector<VKFW::Ref<UniformParameter>> mBindingParameters;
	};

}