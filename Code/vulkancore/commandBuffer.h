#pragma once

#include "../ptr.h"
#include "vk_common.h"
#include "device.h"
#include "commandPool.h"
#include "pipeline.h"


namespace VKFW::vulkancore{
	class CommandBuffer {
	public:
		CommandBuffer(const VKFW::Ref<Device>& device, const VKFW::Ref<CommandPool>& commandPool, bool asSecondary = false);
		~CommandBuffer();

		[[nodiscard]] auto& getCommandBuffer() const { return mCommandBuffer; }

		// VkCommandBufferUsageFlags
		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be submitted once and then freed
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: The command buffer will be used as a secondary command buffer in a render pass
		// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be submitted multiple times

		// VkCommandBufferInheritanceInfo: If the command buffer is a secondary command buffer, then this structure records
		// the information about the render pass and framebuffer that the command buffer will be used in and the inheritance info
		void beginCommandBuffer(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo& inheritance = {});


		// VkSubpassContents:
		// VK_SUBPASS_CONTENTS_INLINE: The command buffer will be recorded inline with the render pass, this command buffer will be used as a primary command buffer
		// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The command buffer will be recorded as a secondary command buffer, this command buffer will be used as a secondary command buffer


		void beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, VkSubpassContents subPassContents = VK_SUBPASS_CONTENTS_INLINE);

		void bindGraphicPipeline(const VKFW::Ref<Pipeline>& pipeline);

		void bindVertexBuffer(const std::vector<VkBuffer>& buffers, uint32_t binding = 0, std::vector<VkDeviceSize> offsets = { 0 });
		void bindIndexBuffer(VkBuffer buffer, uint32_t offset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32);
		void bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet);
		void bindDescriptorSets(const VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
		void pushConstants(
			VkPipelineLayout layout,
			VkShaderStageFlags stageFlags,
			uint32_t offset,
			uint32_t size,
			const void* pData);
		void draw(uint32_t vertexCount);

		void endRenderPass();

		void endCommandBuffer();

		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

		void copyBufferToBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyRegions);

		void copyBufferToImage(const VkBuffer& srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, size_t width, size_t height, bool isCubeMap = false);

		void CopyImageToImage(const VkImage& inSrcImage, VkImage inDstImage, size_t inWidth, size_t inHeight, int inMipmapLevel);

		void CopyRTImageToCubeMap(const VkImage& inSrcImage, VkImage inDstCubeMap, size_t inWidth, size_t inHeight, int inFace, int inMipmapLevel);

		void submitCommandBuffer(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

		void waitCommandBuffer(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

		void transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

	private:
		VkFence mFence = VK_NULL_HANDLE;
		VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };
		VKFW::Ref<CommandPool> mCommandPool{ nullptr };
		VKFW::Ref<Device> mDevice{ nullptr };
	};
}