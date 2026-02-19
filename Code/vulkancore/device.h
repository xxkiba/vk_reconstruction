#pragma once

#include "vk_common.h"
#include "../ptr.h"
#include "instance.h"
#include "windowSurface.h"

namespace VKFW::vulkancore{

	class Device {
	public:
		Device(VKFW::Ref<Instance> instance, VKFW::Ref<WindowSurface> surface);
		~Device();

		void pickPhysicalDevice();

		int rateDevice(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

		void initQueueFamilies(VkPhysicalDevice device);

		void createLogicalDevice();

		bool isQueueFamilyComplete();

		VkSampleCountFlagBits getMaxUsableSampleCount();


		[[nodiscard]] auto getDevice() const { return mDevice; }
		[[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }
		[[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }
		[[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }
		[[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue; }
		[[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }

	private:
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
		VKFW::Ref<Instance> mInstance{ nullptr };
		VKFW::Ref<WindowSurface> mSurface{ nullptr }; // Window surface for rendering

		//Current ID of graphics queue family
		std::optional<uint32_t> mGraphicQueueFamily;
		VkQueue mGraphicQueue{ VK_NULL_HANDLE };

		std::optional<uint32_t> mPresentQueueFamily;
		VkQueue mPresentQueue{ VK_NULL_HANDLE };

		//Logical Device
		VkDevice mDevice{ VK_NULL_HANDLE };

		//Anti-aliasing
		VkSampleCountFlagBits mSampleCounts{ VK_SAMPLE_COUNT_1_BIT }; // Default to 1 sample per pixel
	};
}