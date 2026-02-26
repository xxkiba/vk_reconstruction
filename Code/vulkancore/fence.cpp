#include "fence.h"

namespace VKFW::vulkancore {
	Fence::Fence(const VKFW::Ref<Device>& device, bool signaled) {
		mDevice = device;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
		if (vkCreateFence(mDevice->getDevice(), &fenceInfo, nullptr, &mFence) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create fence!");
		}
	}
	Fence::~Fence() {
		if (mFence != VK_NULL_HANDLE) {
			vkDestroyFence(mDevice->getDevice(), mFence, nullptr);
		}
	}
	void Fence::waitForFence(uint64_t timeout) {
		vkWaitForFences(mDevice->getDevice(), 1, &mFence, VK_TRUE, timeout);
	}
	void Fence::resetFence() {
		vkResetFences(mDevice->getDevice(), 1, &mFence);
	}

}