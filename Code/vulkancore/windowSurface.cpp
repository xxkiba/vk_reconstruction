#include "windowSurface.h"

namespace VKFW::vulkancore {
	WindowSurface::WindowSurface(VKFW::Ref<Instance> instance, VKFW::Ref<VKFW::platform::Window> window) {
		mInstance = instance;
		if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(), nullptr, &mSurface) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create surface");
		}
	}

	WindowSurface::~WindowSurface() {
		vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
		mInstance.reset();
	}
}