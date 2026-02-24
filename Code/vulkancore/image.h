#pragma once
#include "../ptr.h"
#include "vk_common.h"
#include "device.h"
#include "commandPool.h"
#include "commandBuffer.h"
#include "dataBuffer.h"
#include "stb_image.h"

namespace VKFW::vulkancore {
    
    struct ImageDescription {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageType imageType = VK_IMAGE_TYPE_2D;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags usage = 0;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
        bool isCubeMap = false;
    };

    class Image {
    public:
        using Ptr = VKFW::Ref<Image>;

        static VKFW::Ref<Image> Image::createImageFromFile(const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool> commandPool,
            const std::string& filePath,
            VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
            bool flipVertically = false) {
            int texWidth = 0, texHeight = 0, texChannels = 0;

            stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

            stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels || texWidth <= 0 || texHeight <= 0) {
                throw std::runtime_error("Image::createFromFile failed to load image or invalid dimensions! Path: " + filePath);
            }

            const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4; // STBI_rgb_alpha => RGBA8

            // create MapImage
            ImageDescription desc{};
            desc.width = texWidth;
            desc.height = texHeight;
            desc.format = format;
            desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

        static VKFW::Ref<Image> Image::createDepthStencil(const VKFW::Ref<Device>& device, uint32_t width, uint32_t height) {

            VkFormat depthFormat = findDepthFormat(device);

            ImageDescription desc{};
            desc.width = width;
            desc.height = height;
            desc.format = depthFormat;
            desc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            desc.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
                desc.aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            desc.samples = device->getMaxUsableSampleCount();
            return VKFW::MakeRef<Image>(device, desc);
        }


        static VKFW::Ref<Image> Image::createRenderTargetImage(const VKFW::Ref<Device>& device, uint32_t width, uint32_t height, VkFormat inFormat) {
            
            ImageDescription desc{};
            desc.width = width;
            desc.height = height;
            desc.format = inFormat;
            desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            desc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            return VKFW::MakeRef<Image>(device, desc);
        }
        static VKFW::Ref<Image> Image::createMultiSampleImage(const VKFW::Ref<Device>& device, uint32_t width, uint32_t height,VkFormat inFormat) {


            ImageDescription desc{};
            desc.width = width;
            desc.height = height;
            desc.format = inFormat;
            desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            desc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            desc.samples = device->getMaxUsableSampleCount();
            return VKFW::MakeRef<Image>(device, desc);
        }


        Image(const VKFW::Ref<Device>& device, const ImageDescription& desc);
        ~Image();

        void fillImageData(size_t size, const void* pData, const VKFW::Ref<CommandPool>& commandPool, const bool& isCubeMap = false);

        void transitionLayout(VkImageLayout newLayout,
            VkPipelineStageFlags srcStage,
            VkPipelineStageFlags dstStage,
            const VKFW::Ref<CommandPool>& pool,
            const VKFW::Ref<CommandBuffer>& externalCmd = nullptr);

        VkAccessFlags getSrcAccessMask(VkImageLayout oldLayout);
        VkAccessFlags getDstAccessMask(VkImageLayout newLayout);

        void Image::markAsModifiedByHost();
        
        // Getters
        VkImage getHandle() const { return mImage; }
        VkImageView getImageView() const { return mImageView; }
        VkFormat getFormat() const { return mDesc.format; }
        const ImageDescription& getDesc() const { return mDesc; }

        static VkFormat findDepthFormat(const VKFW::Ref<Device>& device, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
            VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            std::vector<VkFormat> depthFormats = {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT
            };
            for (const auto& format : depthFormats) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);
                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                }
                else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            throw std::runtime_error("Error: failed to find supported format!");

        }
    private:
        void createResources();
        void createImageView();
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    private:
        VKFW::Ref<Device> mDevice;
        ImageDescription mDesc;

        VkImage mImage{ VK_NULL_HANDLE };
        VkDeviceMemory mMemory{ VK_NULL_HANDLE };
        VkImageView mImageView{ VK_NULL_HANDLE };
        VkImageLayout mCurrentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
        VkExtent3D mExtent{};

        VkAccessFlags mLastWriteAccess{ 0 };             
        VkPipelineStageFlags mLastWriteStage{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT }; 
    };
}