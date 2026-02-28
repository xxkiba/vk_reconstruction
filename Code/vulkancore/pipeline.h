#pragma once


#include "vk_common.h"
#include "device.h"
#include "shader.h"
#include "renderPass.h"

namespace VKFW::vulkancore {
	class Pipeline {
	public:
		using Ptr = std::shared_ptr<Pipeline>;
		static Ptr create(const VKFW::Ref<Device>& device, const VKFW::Ref<RenderPass>& renderPass) {
			return std::make_shared<Pipeline>(device, renderPass);
		}
		Pipeline(const VKFW::Ref<Device>& device, const VKFW::Ref<RenderPass>& renderPass);
		~Pipeline();
		[[nodiscard]] VkPipeline getPipeline() const { return mPipeline; }
		[[nodiscard]] VkPipelineLayout getPipelineLayout() const { return mLayout; }
		//[[nodiscard]] const VKFW::Ref<RenderPass>& getRenderPass() const { return mRenderPass; }
		//[[nodiscard]] const std::vector<Shader::Ptr>& getShaders() const { return mShaders; }
		void setShaderGroup(const std::vector<VKFW::Ref<Shader>>& shaderGroup);
		void inline setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }
		void inline setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }

		void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment) {
			mBlendAttachmentStates.push_back(blendAttachment);
		}

		void build();
	public:
		VkPipelineVertexInputStateCreateInfo mVertexInputState{};
		VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};
		VkPipelineViewportStateCreateInfo mViewportState{};
		VkPipelineRasterizationStateCreateInfo mRasterState{};
		VkPipelineDynamicStateCreateInfo mDynamicState{};
		VkPipelineMultisampleStateCreateInfo mMultisampleState{};
		std::vector<VkPipelineColorBlendAttachmentState>  mBlendAttachmentStates{};
		VkPipelineColorBlendStateCreateInfo mBlendState{};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};
		VkPipelineLayoutCreateInfo mPipelineLayoutInfo{};
		std::vector<VkViewport> mViewports{};
		std::vector<VkRect2D> mScissors{};
		std::vector<VkDynamicState> dynamicStates = {};


	private:
		VkPipeline mPipeline{ VK_NULL_HANDLE };
		VkPipelineLayout mLayout{ VK_NULL_HANDLE };
		VKFW::Ref<Device> mDevice{ nullptr };
		VKFW::Ref<RenderPass> mRenderPass{ nullptr };
		std::vector<VKFW::Ref<Shader>> mShaders{};
	};
}