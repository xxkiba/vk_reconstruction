#pragma once

#include "../ptr.h"
#include "vk_common.h"
#include "device.h"
#include "commandBuffer.h"
#include "commandPool.h"

namespace VKFW::vulkancore {
	class DataBuffer {
	public:

		static VKFW::Ref<DataBuffer> createVertexBuffer(const VKFW::Ref<Device>& device, VkDeviceSize size, void* pData);
		static VKFW::Ref<DataBuffer> createIndexBuffer(const VKFW::Ref<Device>& device, VkDeviceSize size, void* pData);
		static VKFW::Ref<DataBuffer> createUniformBuffer(const VKFW::Ref<Device>& device, VkDeviceSize size, void* pData = nullptr);
		static VKFW::Ref<DataBuffer> createStageBuffer(const VKFW::Ref<Device>& device, VkDeviceSize size, void* pData = nullptr);


		DataBuffer(const VKFW::Ref<Device>& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~DataBuffer();

		//change memory by Mapping, suitable for Host visible memory
		void updateBufferByMap(const void* data, VkDeviceSize size);
		//If memory is Local optimal, should create StageBuffer, first copy to stage buffer, then copy to this buffer'
		void updateBufferByStage(void* data, VkDeviceSize size);
		void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);
		uint32_t findMemoryType(VKFW::Ref<Device> &device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		[[nodiscard]] VkMemoryPropertyFlags getProperties() const { return mProperties; }

		[[nodiscard]] VkBufferUsageFlags getUsage() const { return mUsage; }
		[[nodiscard]] VkDeviceSize getSize() const { return mSize; }

		[[nodiscard]] VkBuffer getBuffer() const { return mBuffer; }
		[[nodiscard]] VkDeviceMemory getMemory() const { return mMemory; }
		[[nodiscard]] const VkDescriptorBufferInfo& getBufferInfo() const { return mBufferInfo; }

	private:
		VkBuffer mBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory mMemory{ VK_NULL_HANDLE };
		VKFW::Ref<Device> mDevice;
		VkDeviceSize mSize{ 0 };
		VkBufferUsageFlags mUsage{ 0 };
		VkMemoryPropertyFlags mProperties{ 0 };
		VkDescriptorBufferInfo mBufferInfo{};
	};
}