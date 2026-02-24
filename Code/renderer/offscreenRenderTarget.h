#pragma once

#include "../ptr.h"
#include "../vulkancore/device.h"
#include "../vulkancore/renderPass.h"
#include "../factories/renderPassFactory.h"
#include "../vulkancore/image.h"
#include "../vulkancore/commandBuffer.h"
#include "../vulkancore/commandPool.h"

namespace VKFW::renderer {
    class OffscreenRenderTarget {
    public:

        OffscreenRenderTarget(const VKFW::Ref<VKFW::vulkancore::Device>& device,
            const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
            uint32_t width, uint32_t height,
            uint32_t imageCount,
            VkFormat colorFormat,
            VkFormat depthFormat,
            VkImageLayout renderTargetFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        ~OffscreenRenderTarget();
    public:

        void setClearColor(int idx, float r, float g, float b, float a);
        void setClearDepth(float depth, uint32_t stencil);
        void recreate(uint32_t width, uint32_t height); // resize

        VKFW::Ref<VKFW::vulkancore::Image> getRenderTargetImage(int idx) const { return mRenderTargetImages[idx]; }
        const std::vector<VKFW::Ref<VKFW::vulkancore::Image>>& getRenderTargetImages() const { return mRenderTargetImages; }


        std::vector<VkFramebuffer> getOffScreenFramebuffers() const { return mOffScreenFramebuffers; }
        VKFW::Ref<VKFW::vulkancore::RenderPass> getRenderPass() const { return mRenderPass; }

        uint32_t getWidth() const { return mWidth; }
        uint32_t getHeight() const { return mHeight; }


        VkCommandBuffer beginRendering(VkCommandBuffer cmd = nullptr);


    private:
        void createImageEntities();
        void createRenderPass(VkImageLayout renderTargetFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void createFramebuffer();
        void cleanup();

    private:
        VKFW::Ref<VKFW::vulkancore::Device> mDevice;
        VKFW::Ref<VKFW::vulkancore::CommandPool> mCommandPool;
        uint32_t mWidth, mHeight;
        uint32_t mImageCount; // Number of images in the offscreen render target,by default the same as the swap chain image count
        int mColorBufferCount;
        VkFormat mColorFormat, mDepthFormat;

        // Render Target Images
        std::vector<VKFW::Ref<VKFW::vulkancore::Image>> mRenderTargetImages{}; // offscreen render target

        // Depth image for the swapchain
        std::vector<VKFW::Ref<VKFW::vulkancore::Image>> mDepthImages{};

        // Multisampling image for the swapchain, transient image
        std::vector<VKFW::Ref<VKFW::vulkancore::Image>> mMultisampleImages{};


        VKFW::Ref<VKFW::vulkancore::Image> mDepthAttachment;
        std::vector<VkClearValue> mClearValues;
        int mDepthBufferIndex;

        VKFW::Ref<VKFW::vulkancore::RenderPass> mRenderPass{ VK_NULL_HANDLE };
        std::vector<VkFramebuffer> mOffScreenFramebuffers{};

    };
} //namespace VKFW::renderer 