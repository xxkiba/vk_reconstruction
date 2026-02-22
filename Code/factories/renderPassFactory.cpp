// factories/renderPassFactory.cpp
#include "renderPassFactory.h"

namespace VKFW::factories {

    VKFW::Ref<VKFW::vulkancore::RenderPass> RenderPassFactory::CreateSwapchainMsaaPresentPass(
        const VKFW::Ref<VKFW::vulkancore::Device>& device,
        VkFormat swapchainFormat,
        VkSampleCountFlagBits msaaSamples,
        VkFormat depthFormat)
    {
        auto rp = VKFW::MakeRef<VKFW::vulkancore::RenderPass>(device);

        // 0: swapchain final (present)
        VkAttachmentDescription finalAttachment{};
        finalAttachment.format = swapchainFormat;
        finalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        finalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        finalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        finalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        finalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        finalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        finalAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        rp->addAttachment(finalAttachment);

        VkAttachmentReference finalRef{};
        finalRef.attachment = 0;
        finalRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 1: MSAA color
        VkAttachmentDescription msaaAttachment{};
        msaaAttachment.format = swapchainFormat;
        msaaAttachment.samples = msaaSamples;
        msaaAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        msaaAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        msaaAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        msaaAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        msaaAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        msaaAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rp->addAttachment(msaaAttachment);

        VkAttachmentReference msaaRef{};
        msaaRef.attachment = 1;
        msaaRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 2: depth
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        rp->addAttachment(depthAttachment);

        VkAttachmentReference depthRef{};
        depthRef.attachment = 2;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // subpass
        VKFW::vulkancore::SubPass subpass{};
        subpass.addColorAttachmentReference(msaaRef);
        subpass.setResolveAttachmentReference(finalRef);
        subpass.setDepthStencilAttachmentReference(depthRef);

        rp->addSubpass(subpass);

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        rp->addDependency(dependency);

        rp->buildRenderPass();
        return rp;
    }

} // namespace VKFW::factories