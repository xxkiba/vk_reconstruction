#pragma once

#include "../ptr.h"            
#include "vk_common.h"
#include "image.h"
#include "sampler.h"
#include "device.h"
#include "commandPool.h"

#include <array>
#include <string>

namespace VKFW::vulkancore {

    class Texture {
    public:
        using Ref = VKFW::Ref<Texture>;

        static Ref createHDRITexture(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool>& commandPool,
            const std::string& filePath
        ) {
            return VKFW::MakeRef<Texture>(device, commandPool, filePath, VK_FORMAT_R32G32B32A32_SFLOAT);
        }

        static Ref create(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool>& commandPool,
            const std::string& filePath
        ) {
            return VKFW::MakeRef<Texture>(device, commandPool, filePath);
        }

        static Ref create(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool>& commandPool,
            const std::array<std::string, 6>& cubemapPaths
        ) {
            return VKFW::MakeRef<Texture>(device, commandPool, cubemapPaths);
        }

        static Ref createFromImage(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<Image>& image,
            const VKFW::Ref<Sampler>& sampler = nullptr
        ) {
            return VKFW::MakeRef<Texture>(device, image, sampler);
        }

        Texture(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool>& commandPool,
            const std::string& filePath
        );

        Texture(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool>& commandPool,
            const std::string& filePath,
            VkFormat format
        );

        Texture(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<CommandPool>& commandPool,
            const std::array<std::string, 6>& cubemapPaths
        );

        Texture(
            const VKFW::Ref<Device>& device,
            const VKFW::Ref<Image>& image,
            const VKFW::Ref<Sampler>& sampler = nullptr
        );

        ~Texture();

        [[nodiscard]] const VKFW::Ref<Image>& getImage() const { return mImage; }
        [[nodiscard]] const VKFW::Ref<Sampler>& getSampler() const { return mSampler; }

        
        [[nodiscard]] const VkDescriptorImageInfo& getImageInfo() const { return mImageInfo; }
        [[nodiscard]] const std::string& getFilePath() const { return mFilePath; }

    private:
        VKFW::Ref<Image>       mImage{ nullptr };
        VKFW::Ref<Sampler>     mSampler{ nullptr };
        VKFW::Ref<Device>      mDevice{ nullptr };
        VKFW::Ref<CommandPool> mCommandPool{ nullptr };

        VkDescriptorImageInfo  mImageInfo{};
        std::string            mFilePath;
    };

} // namespace VKFW::vulkancore