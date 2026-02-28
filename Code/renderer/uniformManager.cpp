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
	}

	void UniformManager::attachGlobalUniform() {
		auto nvpParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		nvpParam->mBinding = static_cast<uint32_t>(mUniformParameters.size()); // auto-increment binding
		nvpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		nvpParam->mCount = 1;
		nvpParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		nvpParam->mSize = sizeof(NVPMatrices);

		for (int i = 0; i < mFrameCount; i++) {
			auto buffer = VKFW::vulkancore::DataBuffer::createUniformBuffer(mDevice, nvpParam->mSize, nullptr);
			nvpParam->mBuffers.push_back(buffer);
		}

		mUniformParameters.push_back(nvpParam);
	}

	void UniformManager::attachObjectUniform() {
		auto objectParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		objectParam->mBinding = static_cast<uint32_t>(mUniformParameters.size()); // auto-increment binding
		objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectParam->mCount = 1;
		objectParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		objectParam->mSize = sizeof(ObjectUniform);

		for (int i = 0; i < mFrameCount; i++) {
			auto buffer = VKFW::vulkancore::DataBuffer::createUniformBuffer(mDevice, objectParam->mSize, nullptr);
			objectParam->mBuffers.push_back(buffer);
		}

		mUniformParameters.push_back(objectParam);
	}

	void UniformManager::attachCameraUniform() {
		auto camParam = VKFW::MakeRef<VKFW::vulkancore::UniformParameter>();
		camParam->mBinding = static_cast<uint32_t>(mUniformParameters.size());
		camParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camParam->mCount = 1;
		camParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		camParam->mSize = sizeof(cameraParameters);

		for (int i = 0; i < mFrameCount; i++) {
			auto buffer = VKFW::vulkancore::DataBuffer::createUniformBuffer(mDevice, camParam->mSize, nullptr);
			camParam->mBuffers.push_back(buffer);
		}
		mUniformParameters.push_back(camParam);
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
		mUniformParameters[2]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&cameraParams), sizeof(cameraParameters));
	}
	
	void UniformManager::updateUniformBufferByBinding(uint32_t binding, int frameIndex, const void* data, size_t size) {
		for (auto& p : mUniformParameters) {
			if (p && p->mBinding == binding) {
				if (p->mDescriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
					p->mDescriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
					throw std::runtime_error("updateUniformBufferByBinding: binding is not a UBO type.");
				}
				if (frameIndex < 0 || frameIndex >= (int)p->mBuffers.size()) {
					throw std::runtime_error("updateUniformBufferByBinding: frameIndex out of range.");
				}
				if (size > p->mSize) {
					throw std::runtime_error("updateUniformBufferByBinding: size > parameter buffer size.");
				}
				p->mBuffers[frameIndex]->updateBufferByMap(data, size);
				return;
			}
		}
		throw std::runtime_error("updateUniformBufferByBinding: binding not found.");
	}
}
