#include "HDRITools.h"

namespace VKFW::renderer {


	HDRITools::HDRITools(VKFW::Ref<VKFW::vulkancore::Device> device, VKFW::Ref<VKFW::vulkancore::CommandPool> commandPool)
		: mDevice(device), mCommandPool(commandPool) {
		// Initialize the offscreen render target and pipeline if needed
		// mOffscreenRenderTarget = Wrapper::VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(mDevice);
		// mOffscreenPipeline = Wrapper::OffscreenPipeline::create(mDevice);
		InitMatrices();
	}

	HDRITools::~HDRITools() {
		// Cleanup resources if needed
		mImage.reset();
		mSampler.reset();
	}

	void HDRITools::InitMatrices() {
		//gCaptureCameras[0].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//gCaptureCameras[1].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//gCaptureCameras[2].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		//gCaptureCameras[3].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//gCaptureCameras[4].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//gCaptureCameras[5].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

		gCaptureCameras[0].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		gCaptureCameras[1].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		gCaptureCameras[2].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		gCaptureCameras[3].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		gCaptureCameras[4].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		gCaptureCameras[5].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

		for (int i = 0; i < 6; i++) {
			gCaptureCameras[i].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
		}
	}
	void HDRITools::HDRITools2CubeMap(
		const std::string& filePath,
		VKFW::Ref<VKFW::vulkancore::Image>& cubMapImage,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {


		VKFW::Ref<OffscreenRenderTarget> mOffscreenRenderTarget{ nullptr };
		VKFW::Ref<PipelineFactory> factory = PipelineFactory::create(mDevice);
		VKFW::Ref<RenderNode> mOffscreenSphereNode{ nullptr };

		//InitMatrices();
		VKFW::Ref<VKFW::vulkancore::Texture> HDRIToolsTexture = VKFW::vulkancore::Texture::createHDRITexture(mDevice, mCommandPool, filePath);



		mOffscreenRenderTarget = VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(
			mDevice, mCommandPool,
			texWidth, texHeight,
			1,
			VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
			VK_FORMAT_D24_UNORM_S8_UINT, // Depth format
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL // Final layout for the offscreen render target, copy from render target to cubemap image, need to be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		Model::Ptr skyboxModel = Model::create(mDevice);
		skyboxModel->loadModel("assets/skybox_cube.obj",mDevice);
		mOffscreenSphereNode = VKFW::MakeRef<RenderNode>();
		mOffscreenSphereNode->mUniformManager = UniformManager::create();
		mOffscreenSphereNode->mUniformManager->init(mDevice, mCommandPool, 1);

		mOffscreenSphereNode->mUniformManager->attachGlobalUniform();
		mOffscreenSphereNode->mUniformManager->attachObjectUniform();
		mOffscreenSphereNode->mUniformManager->attachCameraUniform();

		mOffscreenSphereNode->mUniformManager->build();
		mOffscreenSphereNode->mModels.push_back(skyboxModel);
		mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));



		mOffscreenSphereNode->mMaterial = Material::create();
		mOffscreenSphereNode->mMaterial->attachImages({ HDRIToolsTexture->getImage() });
		mOffscreenSphereNode->mMaterial->init(mDevice, mCommandPool, 1);

		auto layout0 = mOffscreenSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mOffscreenSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		VKFW::Ref<VKFW::vulkancore::Pipeline> mOffscreenPipeline = factory->build(
			mOffscreenRenderTarget->getRenderPass(),
			texWidth, texHeight,
			inVertShaderPath, inFragShaderPath,
			{ layout0, layout1 },
			mOffscreenSphereNode->mModels[0]->getVertexInputBindingDescriptions(),
			mOffscreenSphereNode->mModels[0]->getAttributeDescriptions(),
			nullptr,
			mDevice->getMaxUsableSampleCount(),
			VK_FRONT_FACE_CLOCKWISE,
			/*needFlipVewport*/ true,
			/*enableDynamicViewPort*/ false
		);


		VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
		offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
		offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[0];
		offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
		offScreenRenderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };
		std::vector<VkClearValue> cvs;
		//0: final output color attachment 1:multisample image 2: depth attachment
		VkClearValue offScreenClearFinalColor{};
		offScreenClearFinalColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearFinalColor);

		//1: Multisample image
		VkClearValue offScreenClearMultiSample{};
		offScreenClearMultiSample.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearMultiSample);

		//2: Depth attachment
		VkClearValue offScreenClearDepth{};
		offScreenClearDepth.depthStencil = { 1.0f, 0 };
		cvs.push_back(offScreenClearDepth);

		offScreenRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(cvs.size());
		offScreenRenderPassBeginInfo.pClearValues = cvs.data();

		for (int i = 0; i < 6; i++) {

			VKFW::Ref<VKFW::vulkancore::CommandBuffer> mCommandBuffer = VKFW::MakeRef<VKFW::vulkancore::CommandBuffer>(mDevice, mCommandPool);


			mNVPMatrices.mViewMatrix = gCaptureCameras[i].getViewMatrix();
			mNVPMatrices.mProjectionMatrix = gCaptureCameras[i].getProjectMatrix();
			mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));
			mCameraParameters.CameraWorldPosition = gCaptureCameras[i].getCamPosition();

			mOffscreenSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mOffscreenSphereNode->mModels[0]->getUniform(), mCameraParameters, 0);


			mCommandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			// Begin offscreen render pass
			mCommandBuffer->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			mCommandBuffer->bindGraphicPipeline(mOffscreenPipeline);
			std::vector<VkDescriptorSet> offscreenDescriptorSets = { 
				mOffscreenSphereNode->mUniformManager->getDescriptorSet(0) ,
				mOffscreenSphereNode->mMaterial->getDescriptorSet(0) 
			};
			mCommandBuffer->bindDescriptorSets(
				mOffscreenPipeline->getPipelineLayout(),
				0,
				static_cast<uint32_t>(offscreenDescriptorSets.size()),
				offscreenDescriptorSets.data());

			mOffscreenSphereNode->draw(mCommandBuffer);
			mCommandBuffer->endRenderPass();

			// Copy the rendered image to the cubemap image
			mCommandBuffer->CopyRTImageToCubeMap(
				mOffscreenRenderTarget->getRenderTargetImages()[0]->getImage(),
				cubMapImage->getImage(),
				texWidth, texHeight, i, 0);


			mCommandBuffer->endCommandBuffer();
			// Submit the command buffer
			mCommandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
			// Wait for the command buffer to finish
			mCommandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

		}

	}

	VKFW::Ref<VKFW::vulkancore::Image> HDRITools::LoadHDRIToolsCubeMapFromFile(
		const VKFW::Ref<VKFW::vulkancore::Device>& device,
		const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
		const std::string& filePath,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {
		// Create the cubemap image
		VKFW::Ref<VKFW::vulkancore::Image> mImage = VKFW::vulkancore::Image::createCubeMapImage(
			mDevice,
			texWidth, texHeight,
			VK_FORMAT_R32G32B32A32_SFLOAT);


		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			mCommandPool);

		InitMatrices();
		// Load the HDR image data
		HDRITools2CubeMap(filePath, mImage, texWidth, texHeight, inVertShaderPath, inFragShaderPath);

		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			mCommandPool);

		return mImage;
	}

	void HDRITools::captureDiffuseIrradianceMap(VKFW::Ref<VKFW::vulkancore::Image>& HDRIToolsCubMapImage,
		VKFW::Ref<VKFW::vulkancore::Image>& diffuseIrradianceCubMapImage,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath,
		std::string inFragShaderPath) {

		VKFW::Ref<OffscreenRenderTarget> mOffscreenRenderTarget{ nullptr };
		VKFW::Ref<PipelineFactory> factory = PipelineFactory::create(mDevice);
		VKFW::Ref<RenderNode> mOffscreenSphereNode{ nullptr };


		mOffscreenRenderTarget = VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(
			mDevice, mCommandPool,
			texWidth, texHeight,
			1,
			VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
			VK_FORMAT_D24_UNORM_S8_UINT, // Depth format
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL // Final layout for the offscreen render target, copy from render target to cubemap image, need to be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		Model::Ptr skyboxModel = Model::create(mDevice);
		skyboxModel->loadModel("assets/skybox_cube.obj",mDevice);

		mOffscreenSphereNode = VKFW::MakeRef<RenderNode>();
		mOffscreenSphereNode->mUniformManager = UniformManager::create();
		mOffscreenSphereNode->mUniformManager->init(mDevice, mCommandPool, 1);
		mOffscreenSphereNode->mUniformManager->attachGlobalUniform();
		mOffscreenSphereNode->mUniformManager->attachObjectUniform();
		mOffscreenSphereNode->mUniformManager->attachCameraUniform();
		mOffscreenSphereNode->mUniformManager->attachCubeMap(HDRIToolsCubMapImage);
		mOffscreenSphereNode->mUniformManager->build();
		mOffscreenSphereNode->mModels.push_back(skyboxModel);
		mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

		std::vector<std::string> textureFiles;
		textureFiles.push_back("assets/book.jpg");
		textureFiles.push_back("assets/diffuse.jpg");
		textureFiles.push_back("assets/metal.jpg");
		mOffscreenSphereNode->mMaterial = Material::create();
		mOffscreenSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mOffscreenSphereNode->mMaterial->init(mDevice, mCommandPool, 1);
		//mOffscreenSphereNode->mMaterial->attachImages({ inCubMapImage });
		auto layout0 = mOffscreenSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mOffscreenSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		VKFW::Ref<VKFW::vulkancore::Pipeline> mOffscreenPipeline = factory->build(
			mOffscreenRenderTarget->getRenderPass(),
			texWidth, texHeight,
			inVertShaderPath, inFragShaderPath,
			{ layout0, layout1 },
			mOffscreenSphereNode->mModels[0]->getVertexInputBindingDescriptions(),
			mOffscreenSphereNode->mModels[0]->getAttributeDescriptions(),
			nullptr,
			mDevice->getMaxUsableSampleCount(),
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			/*needFlipVewport*/ false,
			/*enableDynamicViewPort*/ false
		);


		gCaptureCameras[3].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		gCaptureCameras[2].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
		offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
		offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[0];
		offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
		offScreenRenderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };
		std::vector<VkClearValue> cvs;
		//0: final output color attachment 1:multisample image 2: depth attachment
		VkClearValue offScreenClearFinalColor{};
		offScreenClearFinalColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearFinalColor);

		//1: Multisample image
		VkClearValue offScreenClearMultiSample{};
		offScreenClearMultiSample.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearMultiSample);

		//2: Depth attachment
		VkClearValue offScreenClearDepth{};
		offScreenClearDepth.depthStencil = { 1.0f, 0 };
		cvs.push_back(offScreenClearDepth);

		offScreenRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(cvs.size());
		offScreenRenderPassBeginInfo.pClearValues = cvs.data();

		for (int i = 0; i < 6; i++) {

			VKFW::Ref<VKFW::vulkancore::CommandBuffer> mCommandBuffer = VKFW::MakeRef<VKFW::vulkancore::CommandBuffer>(mDevice, mCommandPool);


			mNVPMatrices.mViewMatrix = gCaptureCameras[i].getViewMatrix();
			mNVPMatrices.mProjectionMatrix = gCaptureCameras[i].getProjectMatrix();
			mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));
			mCameraParameters.CameraWorldPosition = gCaptureCameras[i].getCamPosition();

			mOffscreenSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mOffscreenSphereNode->mModels[0]->getUniform(), mCameraParameters, 0);


			mCommandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			// Begin offscreen render pass
			mCommandBuffer->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			mCommandBuffer->bindGraphicPipeline(mOffscreenPipeline);

			std::vector<VkDescriptorSet> offscreenDescriptorSets = {
				mOffscreenSphereNode->mUniformManager->getDescriptorSet(0),
				mOffscreenSphereNode->mMaterial->getDescriptorSet(0)
			}; 

			mCommandBuffer->bindDescriptorSets(mOffscreenPipeline->getPipelineLayout(), 0, offscreenDescriptorSets.size(), offscreenDescriptorSets.data());

			mOffscreenSphereNode->draw(mCommandBuffer);
			mCommandBuffer->endRenderPass();

			// Copy the rendered image to the cubemap image
			mCommandBuffer->CopyRTImageToCubeMap(
				mOffscreenRenderTarget->getRenderTargetImages()[0]->getImage(),
				diffuseIrradianceCubMapImage->getImage(),
				texWidth, texHeight, i, 0);


			mCommandBuffer->endCommandBuffer();
			// Submit the command buffer
			mCommandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
			// Wait for the command buffer to finish
			mCommandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

		}


	}

	VKFW::Ref<VKFW::vulkancore::Image> HDRITools::generateDiffuseIrradianceMap(
		VKFW::Ref<VKFW::vulkancore::Image> HDRIToolsCubMapImage,
		const VKFW::Ref<VKFW::vulkancore::Device>& device,
		const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {

		// Create the diffuse irradiance map image
		VKFW::Ref<VKFW::vulkancore::Image> mImage = VKFW::vulkancore::Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, true);


		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			mCommandPool);


		// Load the HDR image data
		captureDiffuseIrradianceMap(
			HDRIToolsCubMapImage,
			mImage,
			texWidth, texHeight,
			inVertShaderPath, inFragShaderPath);

		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			mCommandPool);

		return mImage;
	}


	void HDRITools::captureSpecularPrefilterMap(
		VKFW::Ref<VKFW::vulkancore::Image>& HDRIToolsCubMapImage,
		VKFW::Ref<VKFW::vulkancore::Image>& specularPrefilterCubMapImage,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath,
		std::string inFragShaderPath) {

		VKFW::Ref<PipelineFactory> factory = PipelineFactory::create(mDevice);
		VKFW::Ref<RenderNode> mOffscreenSphereNode{ nullptr };


		Model::Ptr skyboxModel = Model::create(mDevice);
		skyboxModel->loadModel("assets/skybox_cube.obj",mDevice);
		mOffscreenSphereNode = VKFW::MakeRef<RenderNode>();
		mOffscreenSphereNode->mUniformManager = UniformManager::create();
		mOffscreenSphereNode->mUniformManager->init(mDevice, mCommandPool, 1);
		mOffscreenSphereNode->mUniformManager->attachGlobalUniform();
		mOffscreenSphereNode->mUniformManager->attachObjectUniform();
		mOffscreenSphereNode->mUniformManager->attachCameraUniform();
		mOffscreenSphereNode->mUniformManager->attachCubeMap(HDRIToolsCubMapImage);
		mOffscreenSphereNode->mUniformManager->build();
		mOffscreenSphereNode->mModels.push_back(skyboxModel);
		mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));
		std::vector<std::string> textureFiles;
		textureFiles.push_back("assets/book.jpg");
		textureFiles.push_back("assets/diffuse.jpg");
		textureFiles.push_back("assets/metal.jpg");
		mOffscreenSphereNode->mMaterial = Material::create();
		mOffscreenSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mOffscreenSphereNode->mMaterial->init(mDevice, mCommandPool, 1);

		gCaptureCameras[3].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		gCaptureCameras[2].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		auto layout0 = mOffscreenSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mOffscreenSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		for (int mipmapLevel = 0; mipmapLevel < 5; mipmapLevel++) {

			uint32_t mipWidth = static_cast<float>(texWidth * std::pow(0.5f, mipmapLevel));
			uint32_t mipHeight = static_cast<float>(texHeight * std::pow(0.5f, mipmapLevel));
			float roughness = static_cast<float>(mipmapLevel) / 4.0f; // Roughness ranges from 0 to 1

			// create offscreen render target for each mipmap level
			VKFW::Ref<OffscreenRenderTarget> mOffscreenRenderTarget = VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(
				mDevice, mCommandPool,
				mipWidth, mipHeight,
				1,
				VK_FORMAT_R32G32B32A32_SFLOAT,// Color format
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			);

			PushConstantManager::Ptr mPushConstantManager = PushConstantManager::create();
			mPushConstantManager->init();
			mPushConstantManager->updateConstantData(
				glm::vec4(0.0f, 0.0f, 0.0f, roughness), // Roughness offset
				glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), // Unused offsets
				glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) // Unused offsets
			);
			mPushConstantManager->setConstantStageFlags(VK_SHADER_STAGE_FRAGMENT_BIT);
			std::vector<VkPushConstantRange> pushConstantRange = { mPushConstantManager->getPushConstantRanges()->getPushConstantRange() };
			// create offscreen pipeline for each mipmap level
			VKFW::Ref<VKFW::vulkancore::Pipeline> mOffscreenPipeline = factory->build(
				mOffscreenRenderTarget->getRenderPass(),
				mipWidth, mipHeight,
				inVertShaderPath, inFragShaderPath,
				{ layout0, layout1 },
				mOffscreenSphereNode->mModels[0]->getVertexInputBindingDescriptions(),
				mOffscreenSphereNode->mModels[0]->getAttributeDescriptions(),
				&pushConstantRange,
				mDevice->getMaxUsableSampleCount(),
				VK_FRONT_FACE_COUNTER_CLOCKWISE,
				/*needFlipVewport*/ false,
				/*enableDynamicViewPort*/ false
			);

			VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
			offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
			offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[0];
			offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
			offScreenRenderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(mipWidth), static_cast<uint32_t>(mipHeight) };
			std::vector<VkClearValue> cvs;
			//0: final output color attachment 1:multisample image 2: depth attachment
			VkClearValue offScreenClearFinalColor{};
			offScreenClearFinalColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };
			cvs.push_back(offScreenClearFinalColor);

			//1: Multisample image
			VkClearValue offScreenClearMultiSample{};
			offScreenClearMultiSample.color = { 0.0f, 0.0f, 0.0f, 0.0f };
			cvs.push_back(offScreenClearMultiSample);

			//2: Depth attachment
			VkClearValue offScreenClearDepth{};
			offScreenClearDepth.depthStencil = { 1.0f, 0 };
			cvs.push_back(offScreenClearDepth);

			offScreenRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(cvs.size());
			offScreenRenderPassBeginInfo.pClearValues = cvs.data();

			for (int i = 0; i < 6; i++) {



				VKFW::Ref<VKFW::vulkancore::CommandBuffer> mCommandBuffer = VKFW::MakeRef<VKFW::vulkancore::CommandBuffer>(mDevice, mCommandPool);

				mNVPMatrices.mViewMatrix = gCaptureCameras[i].getViewMatrix();
				mNVPMatrices.mProjectionMatrix = gCaptureCameras[i].getProjectMatrix();
				mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));
				mCameraParameters.CameraWorldPosition = gCaptureCameras[i].getCamPosition();

				mOffscreenSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mOffscreenSphereNode->mModels[0]->getUniform(), mCameraParameters, 0);


				mCommandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

				// Begin offscreen render pass
				mCommandBuffer->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				mCommandBuffer->bindGraphicPipeline(mOffscreenPipeline);

				std::vector<VkDescriptorSet> offscreenDescriptorSets = {
					mOffscreenSphereNode->mUniformManager->getDescriptorSet(0) ,
					mOffscreenSphereNode->mMaterial->getDescriptorSet(0) };

				mCommandBuffer->bindDescriptorSets(mOffscreenPipeline->getPipelineLayout(), 0, offscreenDescriptorSets.size(), offscreenDescriptorSets.data());
				mCommandBuffer->pushConstants(mOffscreenPipeline->getPipelineLayout(), mPushConstantManager->getConstantParam().stageFlags,
					mPushConstantManager->getConstantParam().offset, mPushConstantManager->getConstantParam().size, &mPushConstantManager->getConstantData());

				mOffscreenSphereNode->draw(mCommandBuffer);
				mCommandBuffer->endRenderPass();

				// Copy the rendered image to the cubemap image
				mCommandBuffer->CopyRTImageToCubeMap(
					mOffscreenRenderTarget->getRenderTargetImages()[0]->getImage(),
					specularPrefilterCubMapImage->getImage(),
					mipWidth, mipHeight, i, mipmapLevel);


				mCommandBuffer->endCommandBuffer();
				// Submit the command buffer
				mCommandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
				// Wait for the command buffer to finish
				mCommandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

			}
		}

	}

	VKFW::Ref<VKFW::vulkancore::Image> HDRITools::generateSpecularPrefilterMap(
		VKFW::Ref<VKFW::vulkancore::Image> HDRIToolsCubMapImage,
		const VKFW::Ref<VKFW::vulkancore::Device>& device,
		const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {
		// Create the specular prefilter map image
		VKFW::Ref<VKFW::vulkancore::Image> mImage = VKFW::vulkancore::Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, true, 5);// 5 mip levels for prefiltering

		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			mCommandPool);
		captureSpecularPrefilterMap(HDRIToolsCubMapImage, mImage, texWidth, texHeight, inVertShaderPath, inFragShaderPath);

		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			mCommandPool);
		return mImage;
	}

	VKFW::Ref<VKFW::vulkancore::Image> HDRITools::generateBRDFLUT(
		const VKFW::Ref<VKFW::vulkancore::Device>& device,
		const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {
		// Create the BRDF LUT image
		VKFW::Ref<VKFW::vulkancore::Image> mImage = VKFW::vulkancore::Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			mCommandPool);

		VKFW::Ref<OffscreenRenderTarget> mOffscreenRenderTarget{ nullptr };
		VKFW::Ref<PipelineFactory> factory = PipelineFactory::create(mDevice);
		VKFW::Ref<RenderNode> mOffscreenSphereNode{ nullptr };


		mOffscreenRenderTarget = VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(
			mDevice, mCommandPool,
			texWidth, texHeight,
			1,
			VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
			VK_FORMAT_D24_UNORM_S8_UINT, // Depth format
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL // Final layout for the offscreen render target, copy from render target to cubemap image, need to be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		Model::Ptr skyboxModel = Model::create(mDevice);
		skyboxModel->loadModel("assets/skybox_cube.obj",mDevice);

		mOffscreenSphereNode = VKFW::MakeRef<RenderNode>();
		mOffscreenSphereNode->mUniformManager = UniformManager::create();
		mOffscreenSphereNode->mUniformManager->init(mDevice, mCommandPool, 1);
		mOffscreenSphereNode->mUniformManager->attachGlobalUniform();
		mOffscreenSphereNode->mUniformManager->attachObjectUniform();
		mOffscreenSphereNode->mUniformManager->attachCameraUniform();
		mOffscreenSphereNode->mUniformManager->build();
		mOffscreenSphereNode->mModels.push_back(skyboxModel);
		mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

		std::vector<std::string> textureFiles;
		textureFiles.push_back("assets/book.jpg");
		textureFiles.push_back("assets/diffuse.jpg");
		textureFiles.push_back("assets/metal.jpg");
		mOffscreenSphereNode->mMaterial = Material::create();
		mOffscreenSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mOffscreenSphereNode->mMaterial->init(mDevice, mCommandPool, 1);
		//mOffscreenSphereNode->mMaterial->attachImages({ inCubMapImage });

		auto layout0 = mOffscreenSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mOffscreenSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		VKFW::Ref<VKFW::vulkancore::Pipeline> mOffscreenPipeline = factory->buildScreenQuadPipeline(
			mOffscreenRenderTarget->getRenderPass(),
			texWidth, texHeight,
			inVertShaderPath, inFragShaderPath,
			{ layout0, layout1 },
			nullptr,
			mDevice->getMaxUsableSampleCount(),
			VK_FRONT_FACE_CLOCKWISE,
			/*needFlipVewport*/ false,
			/*enableDynamicViewPort*/ false
		);



		VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
		offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
		offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[0];
		offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
		offScreenRenderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };
		std::vector<VkClearValue> cvs;
		//0: final output color attachment 1:multisample image 2: depth attachment
		VkClearValue offScreenClearFinalColor{};
		offScreenClearFinalColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearFinalColor);

		//1: Multisample image
		VkClearValue offScreenClearMultiSample{};
		offScreenClearMultiSample.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearMultiSample);

		//2: Depth attachment
		VkClearValue offScreenClearDepth{};
		offScreenClearDepth.depthStencil = { 1.0f, 0 };
		cvs.push_back(offScreenClearDepth);

		offScreenRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(cvs.size());
		offScreenRenderPassBeginInfo.pClearValues = cvs.data();


		VKFW::Ref<VKFW::vulkancore::CommandBuffer> mCommandBuffer = VKFW::MakeRef<VKFW::vulkancore::CommandBuffer>(mDevice, mCommandPool);

		mNVPMatrices.mViewMatrix = gCaptureCameras[0].getViewMatrix();
		mNVPMatrices.mProjectionMatrix = gCaptureCameras[0].getProjectMatrix();
		mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));
		mCameraParameters.CameraWorldPosition = gCaptureCameras[0].getCamPosition();

		mOffscreenSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mOffscreenSphereNode->mModels[0]->getUniform(), mCameraParameters, 0);

		mCommandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// Begin offscreen render pass
		mCommandBuffer->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		mCommandBuffer->bindGraphicPipeline(mOffscreenPipeline);
		std::vector<VkDescriptorSet> offscreenDescriptorSets = { mOffscreenSphereNode->mUniformManager->getDescriptorSet(0) , mOffscreenSphereNode->mMaterial->getDescriptorSet(0) };
		mCommandBuffer->bindDescriptorSets(mOffscreenPipeline->getPipelineLayout(), 0, offscreenDescriptorSets.size(), offscreenDescriptorSets.data());

		mOffscreenSphereNode->draw(mCommandBuffer);
		mCommandBuffer->endRenderPass();
		// Copy the rendered image to the cubemap image
		mCommandBuffer->CopyImageToImage(
			mOffscreenRenderTarget->getRenderTargetImages()[0]->getImage(),
			mImage->getImage(),
			texWidth, texHeight, 0);

		mCommandBuffer->endCommandBuffer();
		// Submit the command buffer
		mCommandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
		// Wait for the command buffer to finish
		mCommandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

		// Set the image layout for shader read
		mImage->transitionLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			mCommandPool);

		return mImage;
	}
}