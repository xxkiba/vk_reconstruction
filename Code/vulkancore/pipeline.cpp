#include "pipeline.h"

namespace VKFW::vulkancore {

	Pipeline::Pipeline(const VKFW::Ref<Device>& device, const VKFW::Ref<RenderPass>& renderPass)
		: mDevice(device), mRenderPass(renderPass) {

	}
	Pipeline::~Pipeline() {
		if (mLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
			mLayout = VK_NULL_HANDLE;
		}
		if (mPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
			mPipeline = VK_NULL_HANDLE;
		}
		mShaders.clear();
		mDevice.reset();
		mRenderPass.reset();
	}
	void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup) {
		mShaders = shaderGroup;
	}
	void Pipeline::build() {
		std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
		for (const auto& shader : mShaders) {
			VkPipelineShaderStageCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderCreateInfo.stage = shader->getShaderStage();
			shaderCreateInfo.module = shader->getShaderModule();
			shaderCreateInfo.pName = shader->getEntryPoint().c_str();

			shaderCreateInfos.push_back(shaderCreateInfo);
		}

		//// Set viewport and scissor
		//mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		//mViewportState.viewportCount = static_cast<uint32_t>(mViewports.size());
		//mViewportState.pViewports = mViewports.data();
		//// Set scissor
		//mViewportState.scissorCount = static_cast<uint32_t>(mScissors.size());
		//mViewportState.pScissors = mScissors.data();

		//// Blending
		//mBlendState.attachmentCount = static_cast<uint32_t>(mBlendAttachmentStates.size());
		//mBlendState.pAttachments = mBlendAttachmentStates.data();

		// Create pipeline layout
		if (mLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
			mLayout = VK_NULL_HANDLE;
		}
		if (vkCreatePipelineLayout(mDevice->getDevice(), &mPipelineLayoutInfo, nullptr, &mLayout) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create pipeline layout!");
		}




		// Create pipeline
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
		pipelineCreateInfo.pStages = shaderCreateInfos.data();
		pipelineCreateInfo.pVertexInputState = &mVertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;
		pipelineCreateInfo.pDynamicState = &mDynamicState; // Dynamic state, if needed
		pipelineCreateInfo.pViewportState = &mViewportState;
		pipelineCreateInfo.pRasterizationState = &mRasterState;
		pipelineCreateInfo.pMultisampleState = &mMultisampleState;
		pipelineCreateInfo.pColorBlendState = &mBlendState;
		pipelineCreateInfo.pDepthStencilState = &mDepthStencilState;// TODO: set depth stencil state
		pipelineCreateInfo.layout = mLayout;
		pipelineCreateInfo.renderPass = mRenderPass->getRenderPass(); // Render pass for the pipeline
		pipelineCreateInfo.subpass = 0;

		// Create pipeline based on existing pipeline if needed
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;


		if (mPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
			mPipeline = VK_NULL_HANDLE;
		}

		// Create pipeline
		// Pipeline cache can be used to cache pipeline creation and can be used to speed up pipeline creation or save into file
		if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create graphics pipeline!");
		}

	}
}