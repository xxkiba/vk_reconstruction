#pragma once

#include "../ptr.h"
#include "vk_common.h"
#include "device.h"
#include "description.h"

namespace VKFW::vulkancore {

	class DescriptorPool {
	public:
		using Ptr = std::shared_ptr<DescriptorPool>;
		static Ptr create(const VKFW::Ref<Device>& device) {
			return std::make_shared<DescriptorPool>(device);
		}
		DescriptorPool(const VKFW::Ref<Device>& device);
		~DescriptorPool();
		void build(const std::vector<VKFW::Ref<UniformParameter>>& params, const int& frameCount);
		void destroyPool();
		[[nodiscard]] auto getDescriptorPool() const {
			return mPool;
		}

	private:
		// need to know the numbers of each uniform buffer in order to save memory
		// the size of memory space is not the size of uniform buffer, but the size of the descriptor
		VkDescriptorPool mPool{ VK_NULL_HANDLE };
		VKFW::Ref<Device> mDevice{ nullptr };
		std::vector<VkDescriptorPoolSize> mPoolSizes{};
		uint32_t mMaxSets{ 0 };
		uint32_t mCurrentSets{ 0 };
		bool mIsCreate{ false };
		bool mIsDestroy{ false };
		bool mIsReset{ false };
		bool mIsAllocate{ false };
		bool mIsFree{ false };
	};
}