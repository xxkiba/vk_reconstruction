#pragma once

#include "vk_common.h"
#include "device.h"

namespace VKFW::vulkancore {

	class Semaphore {
	public:
		using Ptr = std::shared_ptr<Semaphore>;
		static Ptr create(const VKFW::Ref<Device>& device) {
			return std::make_shared<Semaphore>(device);
		}

		Semaphore(const VKFW::Ref<Device>& device);
		~Semaphore();

		[[nodiscard]] VkSemaphore getSemaphore() const { return mSemaphore; }
	private:
		VkSemaphore mSemaphore{ VK_NULL_HANDLE };
		VKFW::Ref<Device> mDevice;
	};
}