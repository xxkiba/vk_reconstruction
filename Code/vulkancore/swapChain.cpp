#include "swapChain.h"
#include "swapChain.h"

namespace VKFW::vulkancore {
	SwapChain::SwapChain(const VKFW::Ref<Device>& device, const VKFW::Ref<VKFW::platform::Window>& window, const VKFW::Ref<WindowSurface>& surface, const VKFW::Ref<CommandPool>& commandPool)
		: mDevice(device), mWindow(window), mSurface(surface), mCommandPool(commandPool) {
		// Initialize swap chain here
		auto swapChainSupportInfo = querySwapChainSupportInfo();
		// Choose the best surface format
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);
		// Choose the best present mode
		VkPresentModeKHR presentMode = choosePresentMode(swapChainSupportInfo.mPresentModes);
		// Choose the swap extent
		VkExtent2D extent = chooseSwapExtent(swapChainSupportInfo.mCapabilities);
		// Set the number of images in the swap chain
		mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;

		// If maxImageCount is not zero and imageCount exceeds maxImageCount, set imageCount to maxImageCount
		// If maxImageCount is zero, it means there is no limit on the number of images
		if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount) {
			mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
		}

		// Create the swap chain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface->getSurface();
		createInfo.minImageCount = mImageCount; // Number of images in the swap chain,might be more than it needs
		createInfo.imageFormat = surfaceFormat.format; // Format of the swap chain images
		createInfo.imageColorSpace = surfaceFormat.colorSpace; // Color space of the swap chain images
		createInfo.imageExtent = extent; // Size of the swap chain images

		// For VR and AR, the layer number is 2, for stereo rendering
		createInfo.imageArrayLayers = 1; // Number of layers in the swap chain images

		// For what purpose the swap chain images are used
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Color attachment for rendering

		// Images in the swap chain can be used for presentation or rendering, but the queue for rendering and presentation may be different
		// so we need to share the images between the two queues
		std::vector<uint32_t>queueFamilyIndices = { mDevice->getGraphicQueueFamily().value(), mDevice->getPresentQueueFamily().value() };
		if (mDevice->getGraphicQueueFamily().value() == mDevice->getPresentQueueFamily().value()) {
			// If the graphics queue family is the same as the present queue family, we can use the same queue for both
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only one queue will use the images
			createInfo.queueFamilyIndexCount = 0; // No need to specify queue family indices
			createInfo.pQueueFamilyIndices = nullptr; // No need to specify queue family indices
		}
		else {
			// If the graphics queue family is different from the present queue family, we need to share the images between the two queues
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Multiple queues will use the images
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()); // Number of queue families that will use the images
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data(); // Queue family indices that will use the images
		}

		// Initialize the transform of the swap chain(rotation, flip, etc.)
		createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform; // No transform
		// Initialize the alpha blending mode
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Opaque composite alpha

		// Initialize the present mode
		createInfo.presentMode = presentMode; // Present mode for the swap chain

		// Initialize the clipping rectangle, if the current window is obstacle, the swap chain will be clipped to the window size,but will influence the callback
		createInfo.clipped = VK_TRUE; // Clipping is enabled

		if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to create swap chain!");
		}

		mSwapChainFormat = surfaceFormat.format; // Set the swap chain format
		mSwapChainExtent = extent; // Set the swap chain extent

		// System might create more images than we need, current image count is the minimum number of images in the swap chain
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr); // Get the number of images in the swap chain
		mSwapChainImages.resize(mImageCount); // Resize the swap chain images vector to hold the images
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data()); // Get the swap chain images

		// Create image views for the swap chain images
		mSwapChainImageViews.resize(mImageCount); // Resize the swap chain image views vector to hold the image views
		for (size_t i = 0; i < mImageCount; i++) {
			mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1); // Create image views for the swap chain images
		}

		// Create depth image
		mDepthImages.resize(mImageCount); // Resize the depth images vector to hold the depth images



		for (size_t i = 0; i < mImageCount; i++) {
			mDepthImages[i] = Image::createDepthStencil(mDevice, extent.width, extent.height);
			mDepthImages[i]->transitionLayout(
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				mCommandPool); // Set the image layout for the depth image
		}

		// Create MultisampleImages
		mMultisampleImages.resize(mImageCount); // Resize the multisample images vector to hold the multisample images


		for (size_t i = 0; i < mImageCount; i++) {
			mMultisampleImages[i] = Image::createMultiSampleImage(
				mDevice,
				mSwapChainExtent.width,
				mSwapChainExtent.height,
				mSwapChainFormat); // Create multisample images for the swap chain

			mMultisampleImages[i]->transitionLayout(
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				mCommandPool); // Set the image layout for the multisample image
		}

	}

	void SwapChain::createFrameBuffers(const VKFW::Ref<RenderPass>& renderPass) {
		// Create framebuffers for the swap chain images
		mSwapChainFramebuffers.resize(mImageCount); // Resize the swap chain framebuffers vector to hold the framebuffers
		for (size_t i = 0; i < mImageCount; i++) {
			// FrameBuffer is a collection of attachments, for eaxample, n color attachments, 1 depth attachment forms a framebuffer
			// These attachments are packed into a framebuffer to send into pipeline
			// Create a framebuffer for each swap chain image
			// Be careful of the order
			std::array<VkImageView, 3> attachments{
				mSwapChainImageViews[i],
				mMultisampleImages[i]->getImageView(), // Multisample image view for the framebuffer
				mDepthImages[i]->getImageView()
			}; // Attachments for the framebuffer
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass->getRenderPass(); // Render pass for the framebuffer
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // Number of attachments in the framebuffer
			framebufferInfo.pAttachments = attachments.data(); // Attachments for the framebuffer
			framebufferInfo.width = mSwapChainExtent.width; // Width of the framebuffer
			framebufferInfo.height = mSwapChainExtent.height; // Height of the framebuffer
			framebufferInfo.layers = 1; // Number of layers in the framebuffer
			if (vkCreateFramebuffer(mDevice->getDevice(), &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Error: Failed to create framebuffer!");
			}
		}
	}

	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image; // Image to create the view for
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Type of the image view
		createInfo.format = format; // Format of the image view

		createInfo.subresourceRange.aspectMask = aspectFlags; // Aspect of the image view
		createInfo.subresourceRange.baseMipLevel = 0; // Base mip level of the image view
		createInfo.subresourceRange.levelCount = mipLevels; // Number of mip levels in the image view
		createInfo.subresourceRange.baseArrayLayer = 0; // Base array layer of the image view
		createInfo.subresourceRange.layerCount = 1; // Number of array layers in the image view

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(mDevice->getDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to create image view in swapchain!");
		}
		return imageView; // Return the created image view
	}

	SwapChain::~SwapChain() {

		for (auto& framebuffer : mSwapChainFramebuffers) {
			vkDestroyFramebuffer(mDevice->getDevice(), framebuffer, nullptr); // Destroy the framebuffers
		}

		for (auto& imageView : mSwapChainImageViews) {
			vkDestroyImageView(mDevice->getDevice(), imageView, nullptr); // Destroy the image views
		}
		if (mSwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
		}
		mSurface.reset();
		mWindow.reset();
		mDevice.reset();
	}
	SwapChainSupportInfo SwapChain::querySwapChainSupportInfo() {
		SwapChainSupportInfo supportInfo;
		// Query swap chain support information
		// Get surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &supportInfo.mCapabilities);
		// Get surface formats and present modes
		uint32_t formatCount;
		// Get the number of available formats
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);
		// If formatCount is not zero, resize the vector to hold the formats
		if (formatCount != 0) {
			supportInfo.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, supportInfo.mFormats.data());
		}
		// Get the number of available present modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);
		// If presentModeCount is not zero, resize the vector to hold the present modes
		if (presentModeCount != 0) {
			supportInfo.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, supportInfo.mPresentModes.data());
		}
		return supportInfo;
	}
	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// If the available formats are undefined, return a custom format
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];

	}
	// Choose the best present mode from the available present modes
	VkPresentModeKHR SwapChain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		// In devices, only FIFO is guaranteed to be available, in mobile devices, for battery saving, FIFO is preferred
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR; // Default to FIFO mode
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return bestMode;
	}

	VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		//If following situation occurs, the system do not allow us to set the size of the swap chain
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		// In the high resolution display, the window size is not equals to the pixel size
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}