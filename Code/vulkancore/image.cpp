#include "image.h"

namespace VKFW::vulkancore {


    

    Image::Image(const VKFW::Ref<Device>& device, const ImageDescription& desc)
        : mDevice(device), mDesc(desc), mCurrentLayout(VK_IMAGE_LAYOUT_UNDEFINED) {
        createResources();
        createImageView();
    }

    Image::~Image() {
        if (mImageView) vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
        if (mImage) vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
        if (mMemory) vkFreeMemory(mDevice->getDevice(), mMemory, nullptr);
    }

    void Image::createResources() {
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = mDesc.imageType;
        imageInfo.extent = { mDesc.width, mDesc.height, mDesc.depth };
        imageInfo.mipLevels = mDesc.mipLevels;
        imageInfo.arrayLayers = mDesc.isCubeMap ? 6 : mDesc.arrayLayers;
        imageInfo.format = mDesc.format;
        imageInfo.tiling = mDesc.tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = mDesc.usage;
        imageInfo.samples = mDesc.samples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (mDesc.isCubeMap) imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (vkCreateImage(mDevice->getDevice(), &imageInfo, nullptr, &mImage) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image");
        }

        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memReq);

        VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, mDesc.properties);

        if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate image memory");
        }
        vkBindImageMemory(mDevice->getDevice(), mImage, mMemory, 0);
    }

    void Image::createImageView() {
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = mImage;
        if (mDesc.isCubeMap) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        else viewInfo.viewType = (mDesc.imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;

        viewInfo.format = mDesc.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = mDesc.aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mDesc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = mDesc.isCubeMap ? 6 : mDesc.arrayLayers;

        if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mImageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }

    void Image::fillImageData(size_t size, const void* pData, const VKFW::Ref<CommandPool>& commandPool, const bool& isCubeMap) {
        assert(pData != nullptr);
        assert(size > 0);

        auto stageBuffer = DataBuffer::createStageBuffer(mDevice, size, (void*)pData);

        auto commandBuffer = VKFW::MakeRef<CommandBuffer>(mDevice, commandPool);
        commandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        commandBuffer->copyBufferToImage(stageBuffer->getBuffer(), mImage, mCurrentLayout, mExtent.width, mExtent.height, isCubeMap);
        commandBuffer->endCommandBuffer();

        commandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
        //commandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());
    }
    

    void Image::transitionLayout( VkImageLayout newLayout,
        VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
        const VKFW::Ref<CommandPool>& pool,const VKFW::Ref<CommandBuffer>& externalCmd) {

        auto executeTransition = [&](const VKFW::Ref<CommandBuffer>& cmd) {
            VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            barrier.oldLayout = mCurrentLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = mImage;
            barrier.subresourceRange = { mDesc.aspectFlags, 0, mDesc.mipLevels, 0, mDesc.isCubeMap ? 6u : 1u };
            barrier.srcAccessMask = getAccessMask(mCurrentLayout);
            barrier.dstAccessMask = getAccessMask(newLayout);

            vkCmdPipelineBarrier(cmd->getCommandBuffer(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            };

        if (externalCmd) {
            executeTransition(externalCmd);
        }
        else {
            auto cmd = VKFW::MakeRef<CommandBuffer>(mDevice, pool);
            cmd->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            executeTransition(cmd);
            cmd->endCommandBuffer();
            cmd->submitCommandBuffer(mDevice->getGraphicQueue());
            
            vkQueueWaitIdle(mDevice->getGraphicQueue());
        }
        mCurrentLayout = newLayout;
    }

    // which operation I have done.
    VkAccessFlags Image::getSrcAccessMask(VkImageLayout oldLayout) {
        if (mLastWriteAccess != 0) {
            return mLastWriteAccess;
        }
        switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return 0;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        default:
            return 0;
        }
    }

    // Which operation I'm gonna do.
    VkAccessFlags Image::getDstAccessMask(VkImageLayout newLayout) {
        switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        default:
            return 0;
        }
    }
    
    void Image::markAsModifiedByHost() {
        // Mark: this image was written by cpu host
        mLastWriteAccess = VK_ACCESS_HOST_WRITE_BIT;
        mLastWriteStage = VK_PIPELINE_STAGE_HOST_BIT;
    }

    uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProp;
        vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProp);
        for (uint32_t i = 0; i < memProp.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties) return i;
        }
        throw std::runtime_error("No suitable memory type");
    }
}