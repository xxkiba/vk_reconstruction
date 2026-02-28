#pragma once
#include "render_common.h"
#include "../vulkancore/image.h"
#include "../vulkancore/sampler.h"
#include "../vulkancore/device.h"
#include "../vulkancore/texture.h"
#include "pushConstantManager.h"
#include "pipelineFactory.h"
#include "offscreenRenderTarget.h"
#include "renderNode.h"
#include "model.h"
#include "Camera.h"


namespace VKFW::renderer {
	class HDRITools {
	public:
		using Ptr = std::shared_ptr<HDRITools>;
		static Ptr create(VKFW::Ref<VKFW::vulkancore::Device> device, VKFW::Ref<VKFW::vulkancore::CommandPool> commandPool) {
			return std::make_shared<HDRITools>(device, commandPool);
		}

		HDRITools(VKFW::Ref<VKFW::vulkancore::Device> device, VKFW::Ref<VKFW::vulkancore::CommandPool> commandPool);
		~HDRITools();

		VKFW::Ref<VKFW::vulkancore::Image> LoadHDRIToolsCubeMapFromFile(
			const VKFW::Ref<VKFW::vulkancore::Device>& device,
			const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
			const std::string& filePath,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);

		void HDRITools2CubeMap(
			const std::string& filePath,
			VKFW::Ref<VKFW::vulkancore::Image>& cubMapImage,
			uint32_t texWidth = 1024, uint32_t texHeight = 1024,
			std::string inVertShaderPath = "shaders/SkyboxVert.spv",
			std::string inFragShaderPath = "shaders/SkyBoxFrag.spv");


		VKFW::Ref<VKFW::vulkancore::Image> generateDiffuseIrradianceMap(
			VKFW::Ref<VKFW::vulkancore::Image> HDRIToolsCubMapImage,
			const VKFW::Ref<VKFW::vulkancore::Device>& device,
			const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);

		void captureDiffuseIrradianceMap(
			VKFW::Ref<VKFW::vulkancore::Image>& HDRIToolsCubMapImage,
			VKFW::Ref<VKFW::vulkancore::Image>& diffuseIrradianceCubMapImage,
			uint32_t texWidth = 32, uint32_t texHeight = 32,
			std::string inVertShaderPath = "shaders/SkyboxVert.spv",
			std::string inFragShaderPath = "shaders/CaptureDiffuseIrradianceFrag.spv");

		VKFW::Ref<VKFW::vulkancore::Image> generateSpecularPrefilterMap(
			VKFW::Ref<VKFW::vulkancore::Image> HDRIToolsCubMapImage,
			const VKFW::Ref<VKFW::vulkancore::Device>& device,
			const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);

		void captureSpecularPrefilterMap(
			VKFW::Ref<VKFW::vulkancore::Image>& HDRIToolsCubMapImage,
			VKFW::Ref<VKFW::vulkancore::Image>& specularPrefilterCubMapImage,
			uint32_t texWidth = 128, uint32_t texHeight = 128,
			std::string inVertShaderPath = "shaders/SkyboxVert.spv",
			std::string inFragShaderPath = "shaders/CaptureSpecularPrefilterFrag.spv");

		VKFW::Ref<VKFW::vulkancore::Image> generateBRDFLUT(
			const VKFW::Ref<VKFW::vulkancore::Device>& device,
			const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);



		void InitMatrices();

	private:
		VKFW::Ref<VKFW::vulkancore::Device> mDevice{ nullptr };
		VKFW::Ref<VKFW::vulkancore::Image> mImage{ nullptr };
		VKFW::Ref<VKFW::vulkancore::Sampler> mSampler{ nullptr };
		VKFW::Ref<VKFW::vulkancore::CommandPool> mCommandPool{ nullptr };

		VkDescriptorImageInfo mImageInfo{};
		std::string mFilePath;
		std::string mVertShaderPath;
		std::string mFragShaderPath;
		NVPMatrices mNVPMatrices{};
		cameraParameters mCameraParameters{};
		Camera gCaptureCameras[6];

	};

}
