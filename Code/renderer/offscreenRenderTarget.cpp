#include "offscreenRenderTarget.h"

namespace VKFW::renderer {
    OffscreenRenderTarget::OffscreenRenderTarget(
        const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
        uint32_t width, uint32_t height,
        uint32_t imageCount,
        VkFormat colorFormat,
        VkFormat depthFormat,
        VkImageLayout renderTargetFinalLayout)
        : mDevice(device), mCommandPool(commandPool), mWidth(width), mHeight(height), mImageCount(imageCount),
        mColorFormat(colorFormat), mDepthFormat(depthFormat)
    {
        createImageEntities();
        createRenderPass(renderTargetFinalLayout);
        createFramebuffer();
    }

    OffscreenRenderTarget::~OffscreenRenderTarget() {
        cleanup();
    }

    void OffscreenRenderTarget::cleanup() {
        for (auto& framebuffer : mOffScreenFramebuffers) {
            vkDestroyFramebuffer(mDevice->getDevice(), framebuffer, nullptr); // Destroy the framebuffers
        }
        mRenderPass.reset();
        mDepthAttachment.reset();
        mClearValues.clear();
    }

    void OffscreenRenderTarget::createImageEntities() {
        mRenderTargetImages.resize(mImageCount);
        mMultisampleImages.resize(mImageCount);
        mDepthImages.resize(mImageCount);


        for (uint32_t i = 0; i < mImageCount; ++i) {

            mRenderTargetImages[i] = VKFW::vulkancore::Image::createRenderTargetImage(
                mDevice,
                mWidth,
                mHeight,
                mColorFormat);

            mRenderTargetImages[i]->transitionLayout(
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                mCommandPool); // Set the image layout for the render target image

            mMultisampleImages[i] = VKFW::vulkancore::Image::createMultiSampleImage(
                mDevice,
                mWidth,
                mHeight,
                mColorFormat); // Create multisample images for the swap chain

            mMultisampleImages[i]->transitionLayout(
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                mCommandPool); // Set the image layout for the multisample image


            mDepthImages[i] = VKFW::vulkancore::Image::createDepthStencil(mDevice, mWidth, mHeight,mDepthFormat);

            mDepthImages[i]->transitionLayout(
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                mCommandPool); // Set the image layout for the depth image



        }
    }


    void OffscreenRenderTarget::createRenderPass(VkImageLayout renderTargetFinalLayout) {
        mRenderPass = VKFW::factories::RenderPassFactory::CreateMsaaRenderPass(
            mDevice,
            mColorFormat,
            mDevice->getMaxUsableSampleCount(),
            mDepthFormat,
            renderTargetFinalLayout);
    }

    void OffscreenRenderTarget::createFramebuffer() {
        // Create framebuffers for the offscreen render target images
        mOffScreenFramebuffers.resize(mImageCount); // Resize the swap chain framebuffers vector to hold the framebuffers
        for (size_t i = 0; i < mImageCount; i++) {
            // FrameBuffer is a collection of attachments, for eaxample, n color attachments, 1 depth attachment forms a framebuffer
            // These attachments are packed into a framebuffer to send into pipeline
            // Create a framebuffer for each swap chain image
            // Be careful of the order
            std::array<VkImageView, 3> attachments{
                mRenderTargetImages[i]->getImageView(), // Render target image view for the framebuffer, serves as a texture for the next renderpass
                mMultisampleImages[i]->getImageView(), // Multisample image view for the framebuffer
                mDepthImages[i]->getImageView()
            }; // Attachments for the framebuffer
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPass->getRenderPass(); // Render pass for the framebuffer
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // Number of attachments in the framebuffer
            framebufferInfo.pAttachments = attachments.data(); // Attachments for the framebuffer
            framebufferInfo.width = mWidth; // Width of the framebuffer
            framebufferInfo.height = mHeight; // Height of the framebuffer
            framebufferInfo.layers = 1; // Number of layers in the framebuffer
            if (vkCreateFramebuffer(mDevice->getDevice(), &framebufferInfo, nullptr, &mOffScreenFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Error: Failed to create offscreen framebuffer!");
            }
        }
    }

    VkCommandBuffer OffscreenRenderTarget::beginRendering(VkCommandBuffer cmd) {
        //TODO: 
        return nullptr;
    }

    void OffscreenRenderTarget::setClearColor(int idx, float r, float g, float b, float a) {
        mClearValues[idx].color = { r,g,b,a };
    }
    void OffscreenRenderTarget::setClearDepth(float depth, uint32_t stencil) {
        mClearValues[mDepthBufferIndex].depthStencil = { depth, stencil };
    }
} //namespace VKFW::renderer
