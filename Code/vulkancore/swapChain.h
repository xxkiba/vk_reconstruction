#pragma once

#include "../ptr.h"
#include "vk_common.h"
#include "../platform/window.h"
#include "windowSurface.h"
#include "renderPass.h"
#include "image.h"
#include "commandPool.h"

#include <sstream>

static const char* VkResultToString(VkResult r) {
	switch (r) {
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_EVENT_SET: return "VK_EVENT_SET";
	case VK_EVENT_RESET: return "VK_EVENT_RESET";
	case VK_INCOMPLETE: return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	default: return "VK_RESULT_UNKNOWN";
	}
}

namespace VKFW::vulkancore {

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR mCapabilities{};
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;
	};

	class SwapChain {
	public:

		SwapChain(const VKFW::Ref<Device>& device, const VKFW::Ref<VKFW::platform::Window>& window, const VKFW::Ref<WindowSurface>& surface, const VKFW::Ref<CommandPool>& commandPool);
		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createFrameBuffers(const VKFW::Ref<RenderPass>& renderPass);

	public:

		[[nodiscard]] auto getSwapChainImageFormat() const { return mSwapChainFormat; }
		[[nodiscard]] auto getSwapChainExtent() const { return mSwapChainExtent; }
		[[nodiscard]] auto getSwapChainImages() const { return mSwapChainImages; }
		[[nodiscard]] auto getSwapChainImageViews() const { return mSwapChainImageViews; }
		[[nodiscard]] auto getSwapChain() const { return mSwapChain; }
		[[nodiscard]] auto getSwapChainFramebuffers() const { return mSwapChainFramebuffers; }
		[[nodiscard]] auto getImageCount() const { return mImageCount; }

		uint32_t acquireNextImage(VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE) {
			uint32_t imageIndex;
			vkAcquireNextImageKHR(mDevice->getDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(), semaphore, fence, &imageIndex);
			return imageIndex;
		}

	private:
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
	private:
		VKFW::Ref<Device> mDevice{ nullptr };
		VKFW::Ref<VKFW::platform::Window> mWindow{ nullptr };
		VKFW::Ref<WindowSurface> mSurface{ nullptr };
		VKFW::Ref<CommandPool> mCommandPool{ nullptr };

		VkFormat mSwapChainFormat;
		VkExtent2D mSwapChainExtent;
		uint32_t mImageCount{ 0 };

		// Swap chain images and image views
		// The swapchain is incharge of creating and destroying the image 
		std::vector<VkImage> mSwapChainImages{};

		// Framework to control the swapchain images
		std::vector<VkImageView> mSwapChainImageViews{};

		// Depth image for the swapchain
		std::vector<Image::Ptr> mDepthImages{};

		// Multisampling image for the swapchain, transient image
		std::vector<Image::Ptr> mMultisampleImages{};


		std::vector<VkFramebuffer> mSwapChainFramebuffers{};

		VkSwapchainKHR mSwapChain{ VK_NULL_HANDLE };
	};
}