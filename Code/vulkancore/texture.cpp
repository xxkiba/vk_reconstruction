#include "texture.h"

namespace VKFW::vulkancore {

    // LDR texture (8-bit) constructor
    Texture::Texture(const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
        const std::string& filePath)
        : mDevice(device), mCommandPool(commandPool), mFilePath(filePath)
    {
        // Use Image factory (no stb usage in Texture anymore)
        mImage = VKFW::vulkancore::Image::createImageFromFile(
            mDevice,
            mCommandPool,
            filePath,
            VK_FORMAT_R8G8B8A8_SRGB,
            /*flipVertically=*/false
        );

        mSampler = VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice);

        mImageInfo.imageLayout = mImage->getLayout();       // should be SHADER_READ_ONLY after factory
        mImageInfo.imageView = mImage->getImageView();
        mImageInfo.sampler = mSampler->getSampler();
    }

    // HDR texture (float) constructor
    Texture::Texture(const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
        const std::string& filePath,
        VkFormat format)
        : mDevice(device), mCommandPool(commandPool), mFilePath(filePath)
    {
        // Your Image HDR factory currently supports only R32G32B32A32_SFLOAT.
        // Keep the same signature for now, and let the Image factory validate/throw.
        mImage = VKFW::vulkancore::Image::createImageFromHDRFile(
            mDevice,
            mCommandPool,
            filePath,
            /*flipVertically=*/false,
            format
        );

        mSampler = VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice);

        mImageInfo.imageLayout = mImage->getLayout();       // should be SHADER_READ_ONLY after factory
        mImageInfo.imageView = mImage->getImageView();
        mImageInfo.sampler = mSampler->getSampler();
    }

    // Cubemap constructor (kept as-is for now; still uses stb in Texture unless you later move this into Image)
    Texture::Texture(const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
        const std::array<std::string, 6>& cubemapPaths)
        : mDevice(device), mCommandPool(commandPool)
    {
        int texWidth = 0, texHeight = 0, texChannels = 0;
        std::vector<stbi_uc*> facePixels(6);
        size_t faceSize = 0;

        // load each face of the cubemap
        for (int i = 0; i < 6; ++i) {
            int w, h, c;
            facePixels[i] = stbi_load(cubemapPaths[i].c_str(), &w, &h, &c, STBI_rgb_alpha);

            if (!facePixels[i] || w == 0 || h == 0) {
                // clean up previously loaded faces
                for (int j = 0; j < i; ++j) {
                    if (facePixels[j]) stbi_image_free(facePixels[j]);
                }
                throw std::runtime_error("Error: failed to load cubemap face: " + cubemapPaths[i]);
            }

            // make sure all faces have the same dimensions
            if (i == 0) {
                texWidth = w;
                texHeight = h;
                texChannels = c;
                faceSize = static_cast<size_t>(w) * static_cast<size_t>(h) * 4; // RGBA
            }
            else {
                if (w != texWidth || h != texHeight) {
                    for (int j = 0; j <= i; ++j) {
                        if (facePixels[j]) stbi_image_free(facePixels[j]);
                    }
                    throw std::runtime_error("Error: cubemap faces must have same dimensions!");
                }
            }
        }

        // create consecutive memory for all faces
        const size_t totalSize = faceSize * 6;
        stbi_uc* allPixels = new stbi_uc[totalSize];

        // Copy each face's pixels into the consecutive memory
        for (int i = 0; i < 6; ++i) {
            memcpy(allPixels + i * faceSize, facePixels[i], faceSize);
            stbi_image_free(facePixels[i]);
        }

        // Create the cubemap image
        mImage = VKFW::vulkancore::Image::createCubeMapImage(
            mDevice,
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            VK_FORMAT_R8G8B8A8_SRGB
        );

        mImage->transitionLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            mCommandPool
        );

        // Fill the image with pixel data
        mImage->fillImageData(totalSize, (void*)allPixels, mCommandPool, true);

        mImage->transitionLayout(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            mCommandPool
        );

        delete[] allPixels;

        // Create the sampler for the cubemap
        mSampler = VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice);

        mImageInfo.imageLayout = mImage->getLayout();
        mImageInfo.imageView = mImage->getImageView();
        mImageInfo.sampler = mSampler->getSampler();
    }

    Texture::Texture(const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::Image>& image,
        const VKFW::Ref<VKFW::vulkancore::Sampler>& sampler)
        : mDevice(device), mImage(image)
    {
        if (!mImage) {
            throw std::runtime_error("Error: VKFW::Ref<VKFW::vulkancore::Image> must not be nullptr!");
        }

        mSampler = sampler ? sampler : VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice);

        mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mImageInfo.imageView = mImage->getImageView();
        mImageInfo.sampler = mSampler->getSampler();
    }

    Texture::~Texture() {
        if (mSampler != nullptr) {
            mSampler.reset();
        }
        if (mImage != nullptr) {
            mImage.reset();
        }
        mDevice = nullptr;
        mCommandPool = nullptr;
    }

} // namespace VKFW::vulkancore