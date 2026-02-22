// factories/renderPassFactory.h
#pragma once
#include "../vulkancore/device.h"
#include "../vulkancore/renderPass.h"

namespace VKFW::factories {

    class RenderPassFactory {
    public:
        static VKFW::Ref<VKFW::vulkancore::RenderPass> CreateSwapchainMsaaPresentPass(
            const VKFW::Ref<VKFW::vulkancore::Device>& device,
            VkFormat swapchainFormat,
            VkSampleCountFlagBits msaaSamples,
            VkFormat depthFormat);
    };

} // namespace VKFW::factories