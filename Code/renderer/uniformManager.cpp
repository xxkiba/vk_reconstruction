#include "uniformManager.h"

namespace VKFW::renderer {
	UniformManager::UniformManager() {
	}

	UniformManager::~UniformManager() {

	}
	void UniformManager::init(const VKFW::Ref<VKFW::vulkancore::Device>& device, const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool, int frameCount) {
		mDevice = device;
		mcommandpool = commandPool;
		mFrameCount = frameCount;
		auto nvpParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		nvpParam->mBinding = 0;
		nvpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		nvpParam->mCount = 1;
		nvpParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		nvpParam->mSize = sizeof(NVPMatrices);

		for (int i = 0; i < frameCount; i++) {
			auto buffer = VKFW::vulkancore::DataBuffer::createUniformBuffer(device, nvpParam->mSize, nullptr);
			nvpParam->mBuffers.push_back(buffer);
		}

		mUniformParameters.push_back(nvpParam);

		auto objectParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		objectParam->mBinding = 1;
		objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectParam->mCount = 1;
		objectParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		objectParam->mSize = sizeof(ObjectUniform);
		for (int i = 0; i < frameCount; i++) {
			auto buffer = VKFW::vulkancore::DataBuffer::createUniformBuffer(device, objectParam->mSize, nullptr);
			objectParam->mBuffers.push_back(buffer);
		}
		mUniformParameters.push_back(objectParam);

		auto textureParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		textureParam->mBinding = 2;
		textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mCount = 1;
		textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		//textureParam->mSize = sizeof(VkDescriptorImageInfo);
		std::array<std::string, 6> cubemapPaths = {
			"assets/px.jpg","assets/nx.jpg",
			"assets/py.jpg","assets/ny.jpg",
			"assets/pz.jpg","assets/nz.jpg"
		};
		textureParam->mTextures.resize(frameCount); // Resize to frameCount, each frame will have its own textures
		for (int i = 0; i < frameCount; i++) {
			auto tex = VKFW::vulkancore::Texture::create(mDevice, mcommandpool, cubemapPaths);
			textureParam->mTextures[i].push_back(tex);
		}
		mUniformParameters.push_back(textureParam);


		auto cameraParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		cameraParam->mBinding = 3;
		cameraParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraParam->mCount = 1;
		cameraParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		cameraParam->mSize = sizeof(cameraParameters);

		for (int i = 0; i < frameCount; i++) {
			auto buffer = VKFW::vulkancore::DataBuffer::createUniformBuffer(device, cameraParam->mSize, nullptr);
			cameraParam->mBuffers.push_back(buffer);
		}

		mUniformParameters.push_back(cameraParam);

	}

	void UniformManager::attachCubeMap(VKFW::Ref<VKFW::vulkancore::Image>& inImage) {
		auto textureParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		textureParam->mBinding = mUniformParameters.size(); // Use the next binding index
		textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mCount = 1;
		textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		auto cubeSampler = VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice, true);

		textureParam->mTextures.resize(mFrameCount); // Resize to frameCount, each frame will have its own textures
		for (int i = 0; i < mFrameCount; i++) {

			auto tex = VKFW::vulkancore::Texture::createFromImage(mDevice, inImage, cubeSampler);
			textureParam->mTextures[i].push_back(tex);
		}
		mUniformParameters.push_back(textureParam);
	}

	void UniformManager::attachImage(VKFW::Ref<VKFW::vulkancore::Image>& inImage) {
		auto textureParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		textureParam->mBinding = mUniformParameters.size(); // Use the next binding index
		textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mCount = 1;
		textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		auto image2DSampler = VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice);

		textureParam->mTextures.resize(mFrameCount); // Resize to frameCount, each frame will have its own textures
		for (int i = 0; i < mFrameCount; i++) {

			auto tex = VKFW::vulkancore::Texture::createFromImage(mDevice, inImage, image2DSampler);
			textureParam->mTextures[i].push_back(tex);
		}
		mUniformParameters.push_back(textureParam);
	}

	void UniformManager::attachMapImage(VKFW::Ref<VKFW::vulkancore::Image>& inImage) {
		auto textureParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		textureParam->mBinding = mUniformParameters.size(); // Use the next binding index
		textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mCount = 1;
		textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		auto MapSampler = VKFW::MakeRef<VKFW::vulkancore::Sampler>(mDevice, false, true);

		textureParam->mTextures.resize(mFrameCount); // Resize to frameCount, each frame will have its own textures
		for (int i = 0; i < mFrameCount; i++) {

			auto tex = VKFW::vulkancore::Texture::createFromImage(mDevice, inImage, MapSampler);
			textureParam->mTextures[i].push_back(tex);
		}
		mUniformParameters.push_back(textureParam);
	}


	void UniformManager::build() {
		mDescriptorLayout = VKFW::vulkancore::DescriptorSetLayout::create(mDevice);
		mDescriptorLayout->build(mUniformParameters);

		mDescriptorPool = VKFW::vulkancore::DescriptorPool::create(mDevice);
		mDescriptorPool->build(mUniformParameters, mFrameCount);

		mDescriptorSet = VKFW::vulkancore::DescriptorSet::create(mDevice, mUniformParameters, mDescriptorLayout, mDescriptorPool, mFrameCount);
	}
	void UniformManager::updateUniformBuffer(const NVPMatrices& vpMatrices, const ObjectUniform& objectUniform, const cameraParameters& cameraParams, const int frameCount) {
		mUniformParameters[0]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&vpMatrices), sizeof(NVPMatrices));
		mUniformParameters[1]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&objectUniform), sizeof(ObjectUniform));

		mUniformParameters[3]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&cameraParams), sizeof(cameraParameters));
	}
}
