# pragma once

#include "vk_common.h"
#include "../ptr.h"

namespace VKFW::vulkancore {
	class Instance {
	public:

		Instance(bool enableValidationLayer);

		~Instance();

		void printAvailableExtensions();
		std::vector<const char*> getRequiredExtensions();

		//layers
		bool checkValidationLayerSupport();
		void setupDebugger();

		[[nodiscard]] VkInstance getInstance() const { return mInstance; }
		[[nodiscard]] bool getEnableValidationLayer() const { return mEnableValidationLayer; }

	private:
		VkInstance mInstance{ VK_NULL_HANDLE };
		bool mEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };
	};
}