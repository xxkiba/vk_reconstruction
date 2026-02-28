#include "pipelineFactory.h"

namespace VKFW::renderer {

    PipelineFactory::PipelineFactory(const VKFW::Ref<VKFW::vulkancore::Device>& device)
        : mDevice(device)
    {
        if (!mDevice) {
            throw std::runtime_error("PipelineFactory: device is null");
        }
    }

    void PipelineFactory::applyViewportScissor(
        const VKFW::Ref<VKFW::vulkancore::Pipeline>& p,
        uint32_t width, uint32_t height,
        bool needFlipViewport)
    {
        VkViewport viewport{};
        if (needFlipViewport) {
            viewport.x = 0.0f;
            viewport.y = static_cast<float>(height);
            viewport.width = static_cast<float>(width);
            viewport.height = -static_cast<float>(height);
        }
        else {
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(width);
            viewport.height = static_cast<float>(height);
        }
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { width, height };

        p->setViewports({ viewport });
        p->setScissors({ scissor });

        p->mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        p->mViewportState.viewportCount = static_cast<uint32_t>(p->mViewports.size());
        p->mViewportState.pViewports = p->mViewports.data();
        p->mViewportState.scissorCount = static_cast<uint32_t>(p->mScissors.size());
        p->mViewportState.pScissors = p->mScissors.data();
    }

    void PipelineFactory::applyDynamicState(
        const VKFW::Ref<VKFW::vulkancore::Pipeline>& p,
        bool enableDynamicViewPort)
    {
        p->mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        if (enableDynamicViewPort) {
            p->mDynamicState.dynamicStateCount = 2;
            p->dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            p->mDynamicState.pDynamicStates = p->dynamicStates.data();
        }
        else {
            p->mDynamicState.dynamicStateCount = 0;
            p->mDynamicState.pDynamicStates = nullptr;
        }
    }

    void PipelineFactory::setupCommonFixedFunction(
        const VKFW::Ref<VKFW::vulkancore::Pipeline>& p,
        VkSampleCountFlagBits sampleCount,
        VkFrontFace inFrontFace,
        bool enableDepthTest,
        bool enableDepthWrite)
    {
        // Input assembly
        p->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        p->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        p->mAssemblyState.primitiveRestartEnable = VK_FALSE;

        // Raster
        p->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        p->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
        p->mRasterState.lineWidth = 1.0f;
        p->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
        p->mRasterState.frontFace = inFrontFace;
        p->mRasterState.rasterizerDiscardEnable = VK_FALSE;
        p->mRasterState.depthClampEnable = VK_FALSE;
        p->mRasterState.depthBiasEnable = VK_FALSE;

        // MSAA
        p->mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        p->mMultisampleState.sampleShadingEnable = VK_FALSE;
        p->mMultisampleState.rasterizationSamples = sampleCount;
        p->mMultisampleState.minSampleShading = 1.0f;
        p->mMultisampleState.pSampleMask = nullptr;
        p->mMultisampleState.alphaToCoverageEnable = VK_FALSE;
        p->mMultisampleState.alphaToOneEnable = VK_FALSE;

        // Depth
        p->mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        p->mDepthStencilState.depthTestEnable = enableDepthTest ? VK_TRUE : VK_FALSE;
        p->mDepthStencilState.depthWriteEnable = enableDepthWrite ? VK_TRUE : VK_FALSE;
        p->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        p->mDepthStencilState.depthBoundsTestEnable = VK_FALSE;
        p->mDepthStencilState.stencilTestEnable = VK_FALSE;
        p->mDepthStencilState.front = {};
        p->mDepthStencilState.back = {};
        p->mDepthStencilState.minDepthBounds = 0.0f;
        p->mDepthStencilState.maxDepthBounds = 1.0f;
    }

    void PipelineFactory::setupBlendDefault(const VKFW::Ref<VKFW::vulkancore::Pipeline>& p)
    {
        VkPipelineColorBlendAttachmentState blend{};
        blend.blendEnable = VK_FALSE;
        blend.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;

        p->pushBlendAttachment(blend);

        p->mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        p->mBlendState.logicOpEnable = VK_FALSE;
        p->mBlendState.logicOp = VK_LOGIC_OP_COPY;
        p->mBlendState.attachmentCount = static_cast<uint32_t>(p->mBlendAttachmentStates.size());
        p->mBlendState.pAttachments = p->mBlendAttachmentStates.data();
        p->mBlendState.blendConstants[0] = 0.0f;
        p->mBlendState.blendConstants[1] = 0.0f;
        p->mBlendState.blendConstants[2] = 0.0f;
        p->mBlendState.blendConstants[3] = 0.0f;
    }

    void PipelineFactory::setupPipelineLayout(
        const VKFW::Ref<VKFW::vulkancore::Pipeline>& p,
        const std::vector<VkDescriptorSetLayout>& setLayouts,
        const std::vector<VkPushConstantRange>* pushConstantRanges)
    {
        p->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        p->mPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        p->mPipelineLayoutInfo.pSetLayouts = setLayouts.data();

        if (pushConstantRanges && !pushConstantRanges->empty()) {
            p->mPipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges->size());
            p->mPipelineLayoutInfo.pPushConstantRanges = pushConstantRanges->data();
        }
        else {
            p->mPipelineLayoutInfo.pushConstantRangeCount = 0;
            p->mPipelineLayoutInfo.pPushConstantRanges = nullptr;
        }
    }

    // ------------------------------------------------------------
    // (1) Mesh pipeline (replacement of OffscreenPipeline::build)
    // ------------------------------------------------------------
    VKFW::Ref<VKFW::vulkancore::Pipeline> PipelineFactory::build(
        const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
        uint32_t width, uint32_t height,
        const std::string& vertexShaderFile,
        const std::string& fragShaderFile,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkVertexInputBindingDescription>& bindingDes,
        const std::vector<VkVertexInputAttributeDescription>& attributeDes,
        const std::vector<VkPushConstantRange>* pushConstantRanges,
        VkSampleCountFlagBits sampleCount,
        VkFrontFace inFrontFace,
        bool needFlipVewport,
        bool enableDynamicViewPort)
    {
        auto p = VKFW::MakeRef<VKFW::vulkancore::Pipeline>(mDevice, renderPass);

        applyViewportScissor(p, width, height, needFlipVewport);
        applyDynamicState(p, enableDynamicViewPort);

        // Shaders
        std::vector<VKFW::Ref<VKFW::vulkancore::Shader>> shaders;
        shaders.push_back(VKFW::MakeRef<VKFW::vulkancore::Shader>(mDevice, vertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT, "main"));
        shaders.push_back(VKFW::MakeRef<VKFW::vulkancore::Shader>(mDevice, fragShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
        p->setShaderGroup(shaders);

        // Vertex input
        p->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        p->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDes.size());
        p->mVertexInputState.pVertexBindingDescriptions = bindingDes.data();
        p->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDes.size());
        p->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

        // Match your OffscreenPipeline defaults: depthTest=true, depthWrite=false
        setupCommonFixedFunction(p, sampleCount, inFrontFace, /*depthTest*/true, /*depthWrite*/false);
        setupBlendDefault(p);
        setupPipelineLayout(p, descriptorSetLayouts, pushConstantRanges);

        p->build();
        return p;
    }

    // ------------------------------------------------------------
    // (2) Fullscreen pipeline (replacement of OffscreenPipeline::buildScreenQuadPipeline)
    // ------------------------------------------------------------
    VKFW::Ref<VKFW::vulkancore::Pipeline> PipelineFactory::buildScreenQuadPipeline(
        const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
        uint32_t width, uint32_t height,
        const std::string& vertexShaderFile,
        const std::string& fragShaderFile,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkPushConstantRange>* pushConstantRanges,
        VkSampleCountFlagBits sampleCount,
        VkFrontFace inFrontFace,
        bool needFlipVewport,
        bool enableDynamicViewPort)
    {
        auto p = VKFW::MakeRef<VKFW::vulkancore::Pipeline>(mDevice, renderPass);

        applyViewportScissor(p, width, height, needFlipVewport);
        applyDynamicState(p, enableDynamicViewPort);

        // Shaders
        std::vector<VKFW::Ref<VKFW::vulkancore::Shader>> shaders;
        shaders.push_back(VKFW::MakeRef<VKFW::vulkancore::Shader>(mDevice, vertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT, "main"));
        shaders.push_back(VKFW::MakeRef<VKFW::vulkancore::Shader>(mDevice, fragShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
        p->setShaderGroup(shaders);

        // Full screen triangle: no vertex input
        p->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        p->mVertexInputState.vertexBindingDescriptionCount = 0;
        p->mVertexInputState.pVertexBindingDescriptions = nullptr;
        p->mVertexInputState.vertexAttributeDescriptionCount = 0;
        p->mVertexInputState.pVertexAttributeDescriptions = nullptr;

        // Match your previous screen-quad: depth off
        setupCommonFixedFunction(p, sampleCount, inFrontFace, /*depthTest*/false, /*depthWrite*/false);
        setupBlendDefault(p);
        setupPipelineLayout(p, descriptorSetLayouts, pushConstantRanges);

        p->build();
        return p;
    }

    // ------------------------------------------------------------
    // (3) HDR capture preset
    // ------------------------------------------------------------
    VKFW::Ref<VKFW::vulkancore::Pipeline> PipelineFactory::buildHDROffscreenCapturePipeline(
        const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
        uint32_t width, uint32_t height,
        const std::string& vertexShaderFile,
        const std::string& fragShaderFile,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkVertexInputBindingDescription>& bindingDes,
        const std::vector<VkVertexInputAttributeDescription>& attributeDes,
        const std::vector<VkPushConstantRange>* pushConstantRanges,
        VkSampleCountFlagBits sampleCount,
        VkFrontFace inFrontFace,
        bool enableDynamicViewPort)
    {
        // HDR capture typically uses non-flipped viewport and depth test on
        auto p = VKFW::MakeRef<VKFW::vulkancore::Pipeline>(mDevice, renderPass);

        applyViewportScissor(p, width, height, /*flip*/false);
        applyDynamicState(p, enableDynamicViewPort);

        std::vector<VKFW::Ref<VKFW::vulkancore::Shader>> shaders;
        shaders.push_back(VKFW::MakeRef<VKFW::vulkancore::Shader>(mDevice, vertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT, "main"));
        shaders.push_back(VKFW::MakeRef<VKFW::vulkancore::Shader>(mDevice, fragShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
        p->setShaderGroup(shaders);

        p->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        p->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDes.size());
        p->mVertexInputState.pVertexBindingDescriptions = bindingDes.data();
        p->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDes.size());
        p->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

        setupCommonFixedFunction(p, sampleCount, inFrontFace, /*depthTest*/true, /*depthWrite*/false);
        setupBlendDefault(p);
        setupPipelineLayout(p, descriptorSetLayouts, pushConstantRanges);

        p->build();
        return p;
    }

    // ------------------------------------------------------------
    // (4) Default screen quad preset
    // ------------------------------------------------------------
    VKFW::Ref<VKFW::vulkancore::Pipeline> PipelineFactory::buildDefaultScreenQuadPipeline(
        const VKFW::Ref<VKFW::vulkancore::RenderPass>& renderPass,
        uint32_t width, uint32_t height,
        const std::string& vertexShaderFile,
        const std::string& fragShaderFile,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkPushConstantRange>* pushConstantRanges,
        VkSampleCountFlagBits sampleCount,
        bool enableDynamicViewPort)
    {
        // Typical full screen: no flip, CLOCKWISE, depth off
        return buildScreenQuadPipeline(
            renderPass,
            width, height,
            vertexShaderFile, fragShaderFile,
            descriptorSetLayouts,
            pushConstantRanges,
            sampleCount,
            VK_FRONT_FACE_CLOCKWISE,
            /*flip*/ false,
            enableDynamicViewPort
        );
    }

} // namespace VKFW::renderer