#pragma once

#include "../ptr.h"
#include "vk_common.h"
#include "instance.h"
#include "../platform/window.h"

namespace VKFW::vulkancore {
	class WindowSurface {

	public:
		WindowSurface(VKFW::Ref<Instance> instance, VKFW::Ref<VKFW::platform::Window> window);
		~WindowSurface();

		[[nodiscard]] auto getSurface() const { return mSurface; }
		[[nodiscard]] auto getInstance() const { return mInstance; }

	private:
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		VKFW::Ref<Instance> mInstance{ nullptr };
	};
}