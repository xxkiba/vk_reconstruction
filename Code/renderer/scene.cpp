// scene.cpp
#include "scene.h"

#include <limits>
#include <stdexcept>

namespace VKFW::renderer {

    Scene::Scene(
        const VKFW::Ref<VKFW::vulkancore::Device>& device,
        const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
        const VKFW::Ref<VKFW::vulkancore::SwapChain>& swapChain,
        const VKFW::Ref<VKFW::vulkancore::RenderPass>& swapChainRenderPass,
        const VKFW::Ref<OffscreenRenderTarget>& offscreenRenderTarget,
        uint32_t width,
        uint32_t height)
        : mDevice(device),
        mCommandPool(commandPool),
        mSwapChain(swapChain),
        mSwapChainRenderPass(swapChainRenderPass),
        mOffscreenRenderTarget(offscreenRenderTarget),
        mWidth(width),
        mHeight(height)
    {
        if (!mDevice || !mCommandPool || !mSwapChain || !mSwapChainRenderPass || !mOffscreenRenderTarget) {
            throw std::runtime_error("Scene: null dependency in constructor");
        }
        mPipelineFactory = PipelineFactory::create(mDevice);
    }

    uint32_t Scene::getImageCount() const {
        return mSwapChain->getImageCount();
    }

    void Scene::ensureDefaultPerFrameUBOs(const UniformManager::Ptr& um) {
        if (!um) return;

        // If your UniformManager::init() is the "old style" (auto-adds UBOs),
        // then build() will already have NVP/Object/Camera.
        //
        // If your UniformManager::init() is the "new empty init" you showed earlier,
        // then you MUST attach UBOs before build().
        //
        // We can't reliably introspect your internal vector here, so the safest approach is:
        // - You add attachGlobalUniform()/attachObjectUniform()
        // - and you must add attachCameraUniform() in UniformManager (recommended)
        //
        // For now, we call the ones you already implemented.
        um->attachGlobalUniform();
        um->attachObjectUniform();
		um->attachCameraUniform(); // you can implement this as a simple wrapper around attachGlobalUniform with a different binding, or implement it properly with its own UBO and binding.

        // TODO (required if your init() is empty-init version):
        //   Add UniformManager::attachCameraUniform() (per-frame cameraParameters UBO)
        //   and call it here.
        //
        // um->attachCameraUniform();
    }

    void Scene::init() {
        initResourcesAndNodes();
        buildPipelines();
        recordCommandBuffers();
    }

    void Scene::initResourcesAndNodes() {
        // -----------------------
        // Cameras (migrated from old initWindow)
        // -----------------------
        mMainCamera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, glm::vec3(0.0f, -0.2f, 1.0f));
        mMainCamera.setPerpective(45.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 1000.0f);
        mMainCamera.setSpeed(0.001f);

        // -----------------------
        // HDRI resources (migrated from old initVulkan HDRI section)
        // -----------------------
        mHDRITools = HDRITools::create(mDevice, mCommandPool);

        // HDRI cubemap
        mHDRICubemap = mHDRITools->LoadHDRIToolsCubeMapFromFile(
            mDevice, mCommandPool,
            "assets/1.hdr",
            512, 512,
            "shaders/HDRI2CubemapVert.spv",
            "shaders/HDRI2CubemapFrag.spv"
        );

        // Diffuse irradiance
        mDiffuseIrradianceMap = mHDRITools->generateDiffuseIrradianceMap(
            mHDRICubemap,
            mDevice, mCommandPool,
            32, 32,
            "shaders/SkyboxVert.spv",
            "shaders/CaptureDiffuseIrradianceFrag.spv"
        );

        // Specular prefilter
        mSpecularPrefilterMap = mHDRITools->generateSpecularPrefilterMap(
            mHDRICubemap,
            mDevice, mCommandPool,
            128, 128,
            "shaders/SkyboxVert.spv",
            "shaders/CaptureSpecularPrefilterFrag.spv"
        );

        // BRDF LUT
        mBRDFLUT = mHDRITools->generateBRDFLUT(
            mDevice, mCommandPool,
            512, 512,
            "shaders/full_screen_triangle.spv",
            "shaders/generateBRDFFrag.spv"
        );

        // -----------------------
        // Nodes creation
        // -----------------------
        mScreenQuadNode = VKFW::MakeRef<RenderNode>();
        mOffscreenPBRNode = VKFW::MakeRef<RenderNode>();
        mSkyboxNode = VKFW::MakeRef<RenderNode>();

        // node cameras (we keep them but tie their behavior to main camera like old code)
        mScreenQuadNode->mCamera = mMainCamera;
        mOffscreenPBRNode->mCamera = mMainCamera;
        mSkyboxNode->mCamera = mMainCamera;

        // -----------------------
        // UniformManagers (migrated)
        // -----------------------
        // ScreenQuad node (samples offscreen images)
        mScreenQuadNode->mUniformManager = UniformManager::create();
        mScreenQuadNode->mUniformManager->init(mDevice, mCommandPool, (int)getImageCount());
        ensureDefaultPerFrameUBOs(mScreenQuadNode->mUniformManager);
        mScreenQuadNode->mUniformManager->build();

        // Skybox node uses cubemap
        mSkyboxNode->mUniformManager = UniformManager::create();
        mSkyboxNode->mUniformManager->init(mDevice, mCommandPool, (int)getImageCount());
        ensureDefaultPerFrameUBOs(mSkyboxNode->mUniformManager);
        mSkyboxNode->mUniformManager->attachCubeMap(mHDRICubemap);
        mSkyboxNode->mUniformManager->build();

        // Offscreen PBR node: attach IBL resources + helmet maps like old
        mOffscreenPBRNode->mUniformManager = UniformManager::create();
        mOffscreenPBRNode->mUniformManager->init(mDevice, mCommandPool, (int)getImageCount());
        ensureDefaultPerFrameUBOs(mOffscreenPBRNode->mUniformManager);

        // IBL resources
        mOffscreenPBRNode->mUniformManager->attachCubeMap(mSpecularPrefilterMap);
        mOffscreenPBRNode->mUniformManager->attachCubeMap(mDiffuseIrradianceMap);
        mOffscreenPBRNode->mUniformManager->attachImage(mBRDFLUT);

        // Helmet maps (migrated from old)
        auto Albedo = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Default_albedo.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        auto Normal = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Default_normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        auto Metallic = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Metallic.png", VK_FORMAT_R8G8B8A8_UNORM);
        auto Roughness = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Roughness.png", VK_FORMAT_R8G8B8A8_UNORM);
        auto AO = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Default_AO.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        auto Emissive = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Default_emissive.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        auto DefaultMR = VKFW::vulkancore::Image::createImageFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Default_metalRoughness.jpg", VK_FORMAT_R8G8B8A8_UNORM);

        mOffscreenPBRNode->mUniformManager->attachMapImage(Albedo);
        mOffscreenPBRNode->mUniformManager->attachMapImage(Normal);
        mOffscreenPBRNode->mUniformManager->attachMapImage(Emissive);
        mOffscreenPBRNode->mUniformManager->attachMapImage(AO);
        mOffscreenPBRNode->mUniformManager->attachMapImage(Metallic);
        mOffscreenPBRNode->mUniformManager->attachMapImage(Roughness);
        mOffscreenPBRNode->mUniformManager->attachMapImage(DefaultMR);

        mOffscreenPBRNode->mUniformManager->build();

        // -----------------------
        // Materials (migrated)
        // -----------------------
        // Offscreen PBR material (dummy textures list + real maps already in uniform manager)
        {
            std::vector<std::string> textureFiles = { "assets/book.jpg", "assets/diffuse.jpg", "assets/metal.jpg" };
            mOffscreenPBRNode->mMaterial = Material::create();
            mOffscreenPBRNode->mMaterial->attachTexturePaths(textureFiles);
            mOffscreenPBRNode->mMaterial->init(mDevice, mCommandPool, (int)getImageCount());
        }

        // Screen quad material samples offscreen render target images
        {
            mScreenQuadNode->mMaterial = Material::create();
            mScreenQuadNode->mMaterial->attachImages(mOffscreenRenderTarget->getRenderTargetImages());
            mScreenQuadNode->mMaterial->init(mDevice, mCommandPool, (int)getImageCount());
        }

        // Skybox usually uses only uniform manager (cubemap). Keep material empty.

        // -----------------------
        // Push constants (kept to match old behavior)
        // -----------------------
        mPushConstantManager = PushConstantManager::create();
        mPushConstantManager->init();

        // -----------------------
        // Models (migrated)
        // -----------------------
        // Screen quad uses fullscreen triangle => no model
        // Offscreen PBR model
        {
            auto offscreenModel = Model::create(mDevice);
            offscreenModel->loadBattleFireModel("assets/DamagedHelmet.staticmesh", mDevice);
            mOffscreenPBRNode->mModels.push_back(offscreenModel);
            mOffscreenPBRNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));
        }
        // Skybox model
        {
            auto skyboxModel = Model::create(mDevice);
            skyboxModel->loadBattleFireComponent("assets/skybox.staticmesh", mDevice);
            mSkyboxNode->mModels.push_back(skyboxModel);
            mSkyboxNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));
        }
    }

    void Scene::buildPipelines() {
        // PBR pipeline renders into OFFSCREEN render pass
        {
            auto layouts = mOffscreenPBRNode->getDescriptorSetLayouts();
            mPBRPipeline = mPipelineFactory->build(
                mOffscreenRenderTarget->getRenderPass(),
                mWidth, mHeight,
                "shaders/pbr1Vert.spv",
                "shaders/pbr1Frag.spv",
                layouts,
                mOffscreenPBRNode->getVertexInputBindingDescriptions(),
                mOffscreenPBRNode->getVertexInputAttributeDescriptions(),
                /*push*/ nullptr, // you used push constants in old createPipeline; keep nullptr here if you want "no push"
                mDevice->getMaxUsableSampleCount(),
                VK_FRONT_FACE_COUNTER_CLOCKWISE,
                /*flip*/ true,
                /*dyn*/ false,
                true,   // depthTest
                true    // depthWrite
            );
        }

        // Screen quad pipeline renders into SWAPCHAIN render pass
        {
            auto layouts = mScreenQuadNode->getDescriptorSetLayouts();
            mScreenQuadPipeline = mPipelineFactory->buildScreenQuadPipeline(
                mSwapChainRenderPass,
                mWidth, mHeight,
                "shaders/full_screen_triangle.spv",
                "shaders/screen_quad.spv",
                layouts,
                /*push*/ nullptr,
                mDevice->getMaxUsableSampleCount(),
                VK_FRONT_FACE_CLOCKWISE,
                /*flip*/ false,
                /*dyn*/ false
            );
        }

        // Skybox pipeline renders into OFFSCREEN render pass
        {
            auto layouts = mSkyboxNode->getDescriptorSetLayouts();
            mSkyboxPipeline = mPipelineFactory->build(
                mOffscreenRenderTarget->getRenderPass(),
                mWidth, mHeight,
                "shaders/SkyboxVert.spv",
                "shaders/SkyBoxFrag.spv",
                layouts,
                mSkyboxNode->getVertexInputBindingDescriptions(),
                mSkyboxNode->getVertexInputAttributeDescriptions(),
                nullptr,
                mDevice->getMaxUsableSampleCount(),
                VK_FRONT_FACE_CLOCKWISE,
                /*flip*/ true,
                /*dyn*/ false,
                true,   // depthTest
                false
            );
        }
    }

    void Scene::recordCommandBuffers() {
        const uint32_t imageCount = getImageCount();
        mCommandBuffers.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            mCommandBuffers[i] = VKFW::MakeRef<VKFW::vulkancore::CommandBuffer>(mDevice, mCommandPool);
        }

        for (uint32_t i = 0; i < imageCount; i++) {
            // Offscreen render pass
            VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
            offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
            offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[i];
            offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
            offScreenRenderPassBeginInfo.renderArea.extent = mSwapChain->getSwapChainExtent();

            std::vector<VkClearValue> offCvs;
            VkClearValue offFinal{}; offFinal.color = { 0,0,0,0 }; offCvs.push_back(offFinal);
            VkClearValue offMS{};    offMS.color = { 0,0,0,0 };    offCvs.push_back(offMS);
            VkClearValue offDepth{}; offDepth.depthStencil = { 1.0f,0 }; offCvs.push_back(offDepth);
            offScreenRenderPassBeginInfo.clearValueCount = (uint32_t)offCvs.size();
            offScreenRenderPassBeginInfo.pClearValues = offCvs.data();

            

            // Begin command buffer
            mCommandBuffers[i]->beginCommandBuffer();

            // ---- Offscreen pass ----
            mCommandBuffers[i]->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Skybox
            mCommandBuffers[i]->bindGraphicPipeline(mSkyboxPipeline);
            {
                std::vector<VkDescriptorSet> sets = { mSkyboxNode->mUniformManager->getDescriptorSet(i) };
                mCommandBuffers[i]->bindDescriptorSets(
                    mSkyboxPipeline->getPipelineLayout(),
                    0,
                    (uint32_t)sets.size(),
                    sets.data());
                mSkyboxNode->draw(mCommandBuffers[i]);
            }

            // PBR object into offscreen
            mCommandBuffers[i]->bindGraphicPipeline(mPBRPipeline);
            {
                std::vector<VkDescriptorSet> sets = {
                    mOffscreenPBRNode->mUniformManager->getDescriptorSet(i),
                    mOffscreenPBRNode->mMaterial->getDescriptorSet(i)
                };
                mCommandBuffers[i]->bindDescriptorSets(
                    mPBRPipeline->getPipelineLayout(),
                    0,
                    (uint32_t)sets.size(),
                    sets.data()
                );
                // keep old push constants behavior if you want:
                // mCommandBuffers[i]->pushConstants(...)

                mOffscreenPBRNode->draw(mCommandBuffers[i]);
            }

            mCommandBuffers[i]->endRenderPass();

            // ---- Swapchain pass ----

            // Swapchain render pass
            VkRenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = mSwapChainRenderPass->getRenderPass();
            renderPassBeginInfo.framebuffer = mSwapChain->getSwapChainFramebuffers()[i];
            renderPassBeginInfo.renderArea.offset = { 0, 0 };
            renderPassBeginInfo.renderArea.extent = mSwapChain->getSwapChainExtent();

            std::vector<VkClearValue> clearValues;
            VkClearValue finalC{}; finalC.color = { 0,0,0,0 }; clearValues.push_back(finalC);
            VkClearValue msC{};    msC.color = { 0,0,0,0 };    clearValues.push_back(msC);
            VkClearValue dC{};     dC.depthStencil = { 1.0f,0 }; clearValues.push_back(dC);
            renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
            renderPassBeginInfo.pClearValues = clearValues.data();

            mCommandBuffers[i]->beginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            mCommandBuffers[i]->bindGraphicPipeline(mScreenQuadPipeline);
            {
                std::vector<VkDescriptorSet> sets = {
                    mScreenQuadNode->mUniformManager->getDescriptorSet(i),
                    mScreenQuadNode->mMaterial->getDescriptorSet(i)
                };
                mCommandBuffers[i]->bindDescriptorSets(
                    mScreenQuadPipeline->getPipelineLayout(),
                    0,
                    (uint32_t)sets.size(),
                    sets.data()
                );

                // fullscreen triangle
                vkCmdDraw(mCommandBuffers[i]->getCommandBuffer(), 3, 1, 0, 0);
            }

            mCommandBuffers[i]->endRenderPass();

            mCommandBuffers[i]->endCommandBuffer();
        }
    }

    void Scene::update(float dt, uint32_t frameIndex) {
        // migrate old camera rotate + uniform update
        mOffscreenPBRNode->mCamera.horizontalRoundRotate(dt, glm::vec3(0.0f), 5.0f, 30.0f);

        mNVPMatrices.mViewMatrix = mOffscreenPBRNode->mCamera.getViewMatrix();
        mNVPMatrices.mProjectionMatrix = mOffscreenPBRNode->mCamera.getProjectMatrix();

        // old code used inverse(modelMatrix) for normal matrix; keep behavior
        if (!mOffscreenPBRNode->mModels.empty() && mOffscreenPBRNode->mModels[0]) {
            mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mOffscreenPBRNode->mModels[0]->getUniform().mModelMatrix));
        }
        mCameraParameters.CameraWorldPosition = mOffscreenPBRNode->mCamera.getCamPosition();

        mOffscreenPBRNode->mUniformManager->updateUniformBuffer(
            mNVPMatrices,
            mOffscreenPBRNode->mModels[0]->getUniform(),
            mCameraParameters,
            (int)frameIndex
        );

        // skybox camera rotate
        mSkyboxNode->mCamera.horizontalRoundRotate(dt, glm::vec3(0.0f), 5.0f, 30.0f);
        mNVPMatrices.mViewMatrix = mSkyboxNode->mCamera.getViewMatrix();
        mNVPMatrices.mProjectionMatrix = mSkyboxNode->mCamera.getProjectMatrix();
        if (!mSkyboxNode->mModels.empty() && mSkyboxNode->mModels[0]) {
            mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mSkyboxNode->mModels[0]->getUniform().mModelMatrix));
            mSkyboxNode->mModels[0]->setModelMatrix(
                glm::translate(glm::mat4(1.0f), glm::vec3(mSkyboxNode->mCamera.getCamPosition()))
            );
        }
        mCameraParameters.CameraWorldPosition = mSkyboxNode->mCamera.getCamPosition();

        mSkyboxNode->mUniformManager->updateUniformBuffer(
            mNVPMatrices,
            mSkyboxNode->mModels[0]->getUniform(),
            mCameraParameters,
            (int)frameIndex
        );

        // If you want screen quad node to also have global/camera uniforms updated, do it here.
    }

    void Scene::onResize(
        const VKFW::Ref<VKFW::vulkancore::SwapChain>& swapChain,
        const VKFW::Ref<VKFW::vulkancore::RenderPass>& swapChainRenderPass,
        const VKFW::Ref<OffscreenRenderTarget>& offscreenRenderTarget,
        uint32_t width,
        uint32_t height)
    {
        mSwapChain = swapChain;
        mSwapChainRenderPass = swapChainRenderPass;
        mOffscreenRenderTarget = offscreenRenderTarget;
        mWidth = width;
        mHeight = height;

        // rebuild screen quad material with new RT images
        mScreenQuadNode->mMaterial = Material::create();
        mScreenQuadNode->mMaterial->attachImages(mOffscreenRenderTarget->getRenderTargetImages());
        mScreenQuadNode->mMaterial->init(mDevice, mCommandPool, (int)getImageCount());

        buildPipelines();
        recordCommandBuffers();
    }

} // namespace VKFW::renderer