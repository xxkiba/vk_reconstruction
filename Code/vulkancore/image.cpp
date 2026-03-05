// image.cpp
#include "image.h"

// stb implementation must be in exactly ONE .cpp in whole program.
// If you also use stb in Texture.cpp, DO NOT define it there again.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdexcept>
#include <vector>
#include <cassert>

namespace VKFW::vulkancore {

    // ------------------------------------------------------------
    // Static helpers: load from file (LDR)
    // ------------------------------------------------------------
    VKFW::Ref<Image> Image::createImageFromFile(
        const VKFW::Ref<Device>& device,
        const VKFW::Ref<CommandPool> commandPool,
        const std::string& filePath,
        VkFormat format,
        bool flipVertically)
    {
        int texWidth = 0, texHeight = 0, texChannels = 0;

        stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels || texWidth <= 0 || texHeight <= 0) {
            throw std::runtime_error("Image::createImageFromFile failed to load image or invalid dimensions! Path: " + filePath);
        }

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4; // RGBA8

        ImageDescription desc{};
        desc.width = static_cast<uint32_t>(texWidth);
        desc.height = static_cast<uint32_t>(texHeight);
        desc.depth = 1;
        desc.mipLevels = 1;
        desc.arrayLayers = 1;
        desc.format = format;
        desc.imageType = VK_IMAGE_TYPE_2D;
        desc.tiling = VK_IMAGE_TILING_OPTIMAL;
        desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        desc.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        desc.samples = VK_SAMPLE_COUNT_1_BIT;
        desc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
        desc.isCubeMap = false;

        auto img = VKFW::MakeRef<Image>(device, desc);

        // UNDEFINED -> TRANSFER_DST
        img->transitionLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            commandPool
        );

        // copy staging -> image
        img->fillImageData(static_cast<size_t>(imageSize), pixels, commandPool, false);

        // TRANSFER_DST -> SHADER_READ_ONLY
        img->transitionLayout(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            commandPool
        );

        stbi_image_free(pixels);
        return img;
    }

    // ------------------------------------------------------------
    // Static helpers: load from HDR file (float)
    // ------------------------------------------------------------
    VKFW::Ref<Image> Image::createImageFromHDRFile(
        const VKFW::Ref<Device>& device,
        const VKFW::Ref<CommandPool> commandPool,
        const std::string& filePath,
        bool flipVertically,
        VkFormat format)
    {
        // Only support float32 RGBA for now
        if (format != VK_FORMAT_R32G32B32A32_SFLOAT) {
            throw std::runtime_error("createImageFromHDRFile: only VK_FORMAT_R32G32B32A32_SFLOAT is supported.");
        }

        int texWidth = 0, texHeight = 0, texChannels = 0;
        stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

        // load float image (keep original channels)
        float* pixels = stbi_loadf(filePath.c_str(), &texWidth, &texHeight, &texChannels, 0);
        if (!pixels || texWidth <= 0 || texHeight <= 0) {
            throw std::runtime_error("Image::createImageFromHDRFile failed to load HDR or invalid dimensions! Path: " + filePath);
        }

        // pack to RGBA float, A = 1.0 if absent
        const int pixelCount = texWidth * texHeight;
        std::vector<float> floatRGBA(pixelCount * 4);

        for (int i = 0; i < pixelCount; ++i) {
            floatRGBA[i * 4 + 0] = (texChannels > 0) ? pixels[i * texChannels + 0] : 0.0f;
            floatRGBA[i * 4 + 1] = (texChannels > 1) ? pixels[i * texChannels + 1] : 0.0f;
            floatRGBA[i * 4 + 2] = (texChannels > 2) ? pixels[i * texChannels + 2] : 0.0f;
            floatRGBA[i * 4 + 3] = (texChannels > 3) ? pixels[i * texChannels + 3] : 1.0f;
        }

        ImageDescription desc{};
        desc.width = static_cast<uint32_t>(texWidth);
        desc.height = static_cast<uint32_t>(texHeight);
        desc.depth = 1;
        desc.mipLevels = 1;
        desc.arrayLayers = 1;
        desc.format = format;
        desc.imageType = VK_IMAGE_TYPE_2D;
        desc.tiling = VK_IMAGE_TILING_OPTIMAL;
        desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        desc.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        desc.samples = VK_SAMPLE_COUNT_1_BIT;
        desc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
        desc.isCubeMap = false;

        auto img = VKFW::MakeRef<Image>(device, desc);

        img->transitionLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            commandPool
        );

        const VkDeviceSize byteSize = static_cast<VkDeviceSize>(floatRGBA.size() * sizeof(float));
        img->fillImageData(static_cast<size_t>(byteSize), floatRGBA.data(), commandPool, false);

        img->transitionLayout(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            commandPool
        );

        stbi_image_free(pixels);
        return img;
    }

    // ------------------------------------------------------------
    // Image lifecycle
    // ------------------------------------------------------------
    Image::Image(const VKFW::Ref<Device>& device, const ImageDescription& desc)
        : mDevice(device), mDesc(desc), mCurrentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
    {
        if (!mDevice) {
            throw std::runtime_error("Image: device is null");
        }
        createResources();
        createImageView();
    }

    Image::~Image() {
        if (mImageView) vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
        if (mImage) vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
        if (mMemory) vkFreeMemory(mDevice->getDevice(), mMemory, nullptr);
    }

    void Image::createResources() {
        // IMPORTANT: set extent for later uploads
        mExtent = { mDesc.width, mDesc.height, mDesc.depth };

        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = mDesc.imageType;
        imageInfo.extent = { mDesc.width, mDesc.height, mDesc.depth };
        imageInfo.mipLevels = mDesc.mipLevels;
        imageInfo.arrayLayers = mDesc.isCubeMap ? 6u : mDesc.arrayLayers;
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

        VkMemoryRequirements memReq{};
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

        if (mDesc.isCubeMap) {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }
        else {
            viewInfo.viewType = (mDesc.imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
        }

        viewInfo.format = mDesc.format;
        viewInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };

        viewInfo.subresourceRange.aspectMask = mDesc.aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mDesc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = mDesc.isCubeMap ? 6u : mDesc.arrayLayers;

        if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mImageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }

    uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProp{};
        vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProp);

        for (uint32_t i = 0; i < memProp.memoryTypeCount; i++) {
            if ((typeFilter & (1u << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Image::findMemoryType: no suitable memory type");
    }

    // ------------------------------------------------------------
    // Data upload
    // ------------------------------------------------------------
    void Image::fillImageData(size_t size, const void* pData, const VKFW::Ref<CommandPool>& commandPool, const bool& isCubeMap) {
        assert(pData != nullptr);
        assert(size > 0);

        auto stageBuffer = DataBuffer::createStageBuffer(mDevice, size, (void*)pData);

        auto commandBuffer = VKFW::MakeRef<CommandBuffer>(mDevice, commandPool);
        commandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        commandBuffer->copyBufferToImage(
            stageBuffer->getBuffer(),
            mImage,
            mCurrentLayout,
            mExtent.width,
            mExtent.height,
            isCubeMap
        );

        commandBuffer->endCommandBuffer();
        commandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
        // commandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());
    }

    // ------------------------------------------------------------
    // Layout transitions / barriers
    // ------------------------------------------------------------
    void Image::transitionLayout(
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage,
        const VKFW::Ref<CommandPool>& pool,
        const VKFW::Ref<CommandBuffer>& externalCmd)
    {
        auto executeTransition = [&](const VKFW::Ref<CommandBuffer>& cmd) {
            VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            barrier.oldLayout = mCurrentLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = mImage;

            barrier.subresourceRange.aspectMask = mDesc.aspectFlags;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = mDesc.mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = mDesc.isCubeMap ? 6u : mDesc.arrayLayers;

            barrier.srcAccessMask = getSrcAccessMask(mCurrentLayout);
            barrier.dstAccessMask = getDstAccessMask(newLayout);

            vkCmdPipelineBarrier(
                cmd->getCommandBuffer(),
                srcStage,
                dstStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
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
        mLastWriteAccess = VK_ACCESS_HOST_WRITE_BIT;
        mLastWriteStage = VK_PIPELINE_STAGE_HOST_BIT;
    }

} // namespace VKFW::vulkancore