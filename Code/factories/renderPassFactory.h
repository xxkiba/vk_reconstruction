// factories/renderPassFactory.h
#pragma once
#include "../vulkancore/device.h"
#include "../vulkancore/renderPass.h"

namespace VKFW::factories {

    class RenderPassFactory {
    public:
        static VKFW::Ref<VKFW::vulkancore::RenderPass> CreateMsaaRenderPass(
            const VKFW::Ref<VKFW::vulkancore::Device>& device,
            VkFormat mColorFormat,
            VkSampleCountFlagBits msaaSamples,
            VkFormat depthFormat,
            VkImageLayout imageFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    };

} // namespace VKFW::factories