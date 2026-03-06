#pragma once
#include "render_common.h"
#include "../vulkancore/device.h"
#include "../vulkancore/pipeline.h"
#include "../vulkancore/renderPass.h"
#include "../vulkancore/shader.h"

namespace VKFW::renderer {

    class PipelineFactory {
    public:
        using Ptr = VKFW::Ref<PipelineFactory>;

        static Ptr create(const VKFW::Ref<VKFW::vulkancore::Device>& device) {
            return VKFW::MakeRef<PipelineFactory>(device);
        }

        explicit PipelineFactory(const VKFW::Ref<VKFW::vulkancore::Device>& device);

        // ------------------------------------------------------------
        // (1) Mesh pipeline: replacement of OffscreenPipeline::build
        // Signature kept the same as your OffscreenPipeline::build (except device removed because factory owns it)
        // ------------------------------------------------------------
        VKFW::Ref<VKFW::vulkancore::Pipeline> build(
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
            uint32_t width, uint32_t height,
            const std::string& vertexShaderFile,
            const std::string& fragShaderFile,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkVertexInputBindingDescription>& bindingDes,
            const std::vector<VkVertexInputAttributeDescription>& attributeDes,
            const std::vector<VkPushConstantRange>* pushConstantRanges = nullptr,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
            VkFrontFace inFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            bool needFlipVewport = true,
            bool enableDynamicViewPort = false,
            bool enableDepthTest = true,
            bool enableDepthWrite = true
        );

        // ------------------------------------------------------------
        // (2) Fullscreen pipeline: replacement of OffscreenPipeline::buildScreenQuadPipeline
        // Signature kept the same as your OffscreenPipeline::buildScreenQuadPipeline (except device removed)
        // ------------------------------------------------------------
        VKFW::Ref<VKFW::vulkancore::Pipeline> buildScreenQuadPipeline(
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
            uint32_t width, uint32_t height,
            const std::string& vertexShaderFile,
            const std::string& fragShaderFile,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkPushConstantRange>* pushConstantRanges = nullptr,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
            VkFrontFace inFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            bool needFlipVewport = true,
            bool enableDynamicViewPort = false,
            bool enableDepthTest = false,
            bool enableDepthWrite = false
        );

        // ------------------------------------------------------------
        // (3) Preset: HDR capture offscreen (typical for cubemap/irradiance/prefilter capture)
        // Mesh pipeline with:
        //   - needFlipVewport = false
        //   - depthTest = true, depthWrite = false
        // ------------------------------------------------------------
        VKFW::Ref<VKFW::vulkancore::Pipeline> buildHDROffscreenCapturePipeline(
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
            uint32_t width, uint32_t height,
            const std::string& vertexShaderFile,
            const std::string& fragShaderFile,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkVertexInputBindingDescription>& bindingDes,
            const std::vector<VkVertexInputAttributeDescription>& attributeDes,
            const std::vector<VkPushConstantRange>* pushConstantRanges = nullptr,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
            VkFrontFace inFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            bool enableDynamicViewPort = false
        );

        // ------------------------------------------------------------
        // (4) Preset: Default screen quad (fullscreen triangle)
        // Fullscreen pipeline with:
        //   - frontFace = CLOCKWISE
        //   - needFlipVewport = false
        //   - depthTest = false, depthWrite = false
        // ------------------------------------------------------------
        VKFW::Ref<VKFW::vulkancore::Pipeline> buildDefaultScreenQuadPipeline(
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
            uint32_t width, uint32_t height,
            const std::string& vertexShaderFile,
            const std::string& fragShaderFile,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkPushConstantRange>* pushConstantRanges = nullptr,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
            bool enableDynamicViewPort = false
        );

    private:
        void applyViewportScissor(
            const VKFW::Ref<VKFW::vulkancore::Pipeline>& pipeline,
            uint32_t width, uint32_t height,
            bool needFlipViewport);

        void applyDynamicState(
            const VKFW::Ref<VKFW::vulkancore::Pipeline>& pipeline,
            bool enableDynamicViewPort);

        void setupCommonFixedFunction(
            const VKFW::Ref<VKFW::vulkancore::Pipeline>& pipeline,
            VkSampleCountFlagBits sampleCount,
            VkFrontFace inFrontFace,
            bool enableDepthTest,
            bool enableDepthWrite);

        void setupBlendDefault(
            const VKFW::Ref<VKFW::vulkancore::Pipeline>& pipeline);

        void setupPipelineLayout(
            const VKFW::Ref<VKFW::vulkancore::Pipeline>& pipeline,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkPushConstantRange>* pushConstantRanges);

    private:
        VKFW::Ref<VKFW::vulkancore::Device> mDevice{ nullptr };
    };

} // namespace VKFW::renderer