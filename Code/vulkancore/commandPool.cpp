#include "commandPool.h"

namespace VKFW::vulkancore {
	CommandPool::CommandPool(const VKFW::Ref<Device>& device, VkCommandPoolCreateFlagBits flags)
		: mDevice(device) {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = mDevice->getGraphicQueueFamily().value();

		// Specify the command pool's attributes, memory management, and usage
		// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Command buffers can be reset individually
		// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Command buffers are short-lived and can be freed quickly, can not be reset individually, must be reset together via vkResetCommandPool
		poolInfo.flags = flags;
		if (vkCreateCommandPool(mDevice->getDevice(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to create command pool!");
		}
	}
	CommandPool::~CommandPool() {
		if (mCommandBuffer != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool, 1, &mCommandBuffer);
		}
		if (mCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
		}
	}
}