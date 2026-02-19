#pragma once
#include "vk_common.h"
#include "../ptr.h"
#include "device.h"

namespace VKFW::vulkancore {
	class CommandPool {
	public:

		CommandPool(const VKFW::Ref<Device>& device, VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		~CommandPool();

		[[nodiscard]] auto getCommandPool() const { return mCommandPool; }

	private:
		VkCommandPool mCommandPool{ VK_NULL_HANDLE };
		VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };
		VKFW::Ref<Device> mDevice{ nullptr };
		uint32_t mQueueFamilyIndex{ 0 };
	};
}