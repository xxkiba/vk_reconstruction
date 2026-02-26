#include "Material.h"

namespace VKFW::renderer {
    Material::Material() {
        mTexturePaths.clear();
        mAttachedImages.clear();
        mTextures.clear();
    }


    void Material::attachTexturePath(const std::string& path) {
        mTexturePaths.push_back(path);
    }

    void Material::attachTexturePaths(const std::vector<std::string>& paths) {
        mTexturePaths.insert(mTexturePaths.end(), paths.begin(), paths.end());
    }


    void Material::attachImages(const std::vector<VKFW::Ref<VKFW::vulkancore::Image>>& perFrameImages) {
        mAttachedImagesPerFrame.push_back(perFrameImages);
    }
    void Material::init(const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
        int frameCount) {


        std::vector<VKFW::Ref<VKFW::vulkancore::UniformParameter>> params;


        // 1. Create uniform parameter for the texture
        auto textureParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
        textureParam->mBinding = 0;
        textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureParam->mCount = static_cast<uint32_t>(mTexturePaths.size() + mAttachedImagesPerFrame.size()); // Number of textures
        textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // 2. Create textures for each frame
        textureParam->mTextures.resize(frameCount); // Resize to frameCount, each frame will have its own textures
        for (int i = 0; i < frameCount; i++) {
            for (const auto& path : mTexturePaths) {
                textureParam->mTextures[i].push_back(VKFW::vulkancore::Texture::create(device, commandPool, path));
            }
            for (const auto& images : mAttachedImagesPerFrame) {
                textureParam->mTextures[i].push_back(VKFW::vulkancore::Texture::createFromImage(device, images[i]));
            }
        }
        params.push_back(textureParam);


        // 3. Create descriptor set layout
        mDescriptorLayout = VKFW::MakeRef<VKFW::vulkancore::DescriptorSetLayout>(device);
        mDescriptorLayout->build(params);

        // 4. Create descriptor pool
        mDescriptorPool = VKFW::MakeRef<VKFW::vulkancore::DescriptorPool>(device);
        mDescriptorPool->build(params, frameCount);

        // 5. Create descriptor set
        mDescriptorSet = VKFW::MakeRef<VKFW::vulkancore::DescriptorSet>(device, params, mDescriptorLayout, mDescriptorPool, frameCount);
    }

    Material::~Material() {
        // Resource release if needed (smart pointers usually handle this)
    }
}

