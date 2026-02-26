#pragma once
#include "../vulkancore/texture.h"
#include "../vulkancore/descriptorSetLayout.h"
#include "../vulkancore/descriptorPool.h"
#include "../vulkancore/descriptorSet.h"
#include "../vulkancore/device.h"
#include "../vulkancore/commandPool.h"

namespace VKFW::renderer {
    class Material {
    public:
        using Ptr = std::shared_ptr<Material>;
        static Ptr create() {
            return std::make_shared<Material>();
        }

        Material();
        ~Material();

        void init(const VKFW::Ref<VKFW::vulkancore::Device>& device,
            const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
            int frameCount);

        void attachTexturePath(const std::string& path);

        void attachTexturePaths(const std::vector<std::string>& paths);

        void attachImages(const std::vector<VKFW::Ref<VKFW::vulkancore::Image>>& perFrameImages);

        [[nodiscard]] auto getDescriptorLayout() const {
            return mDescriptorLayout;
        }
        [[nodiscard]] auto getDescriptorPool() const {
            return mDescriptorPool;
        }
        [[nodiscard]] auto getDescriptorSet(int frameCount) const {
            return mDescriptorSet->getDescriptorSet(frameCount);
        }

    private:
        std::vector<std::string> mTexturePaths; // strings used to create textures
        std::vector<VKFW::Ref<VKFW::vulkancore::Image>> mAttachedImages; // Images used to create textures
        std::vector<std::vector<VKFW::Ref<VKFW::vulkancore::Image>>> mAttachedImagesPerFrame;
        std::vector<VKFW::Ref<VKFW::vulkancore::Texture>> mTextures;
        VKFW::Ref<VKFW::vulkancore::DescriptorSetLayout> mDescriptorLayout{ nullptr };
        VKFW::Ref<VKFW::vulkancore::DescriptorPool> mDescriptorPool{ nullptr };
        VKFW::Ref<VKFW::vulkancore::DescriptorSet> mDescriptorSet{ nullptr };
    };
}

