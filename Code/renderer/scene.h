// scene.h
#pragma once

#include "render_common.h"
#include "../ptr.h"

#include "../vulkancore/device.h"
#include "../vulkancore/commandPool.h"
#include "../vulkancore/swapChain.h"
#include "../vulkancore/renderPass.h"
#include "../vulkancore/commandBuffer.h"
#include "../vulkancore/semaphore.h"
#include "../vulkancore/fence.h"

#include "offscreenRenderTarget.h"
#include "pipelineFactory.h"
#include "renderNode.h"
#include "HDRITools.h"
#include "pushConstantManager.h"

namespace VKFW::renderer {

    class Scene {
    public:
        using Ptr = VKFW::Ref<Scene>;
        static Ptr create(
            const VKFW::Ref<VKFW::vulkancore::Device>& device,
            const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
            const VKFW::Ref<VKFW::vulkancore::SwapChain>& swapChain,
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& swapChainRenderPass,
            const VKFW::Ref<OffscreenRenderTarget>& offscreenRenderTarget,
            uint32_t width,
            uint32_t height)
        {
            return VKFW::MakeRef<Scene>(device, commandPool, swapChain, swapChainRenderPass, offscreenRenderTarget, width, height);
        }

        Scene(
            const VKFW::Ref<VKFW::vulkancore::Device>& device,
            const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool,
            const VKFW::Ref<VKFW::vulkancore::SwapChain>& swapChain,
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& swapChainRenderPass,
            const VKFW::Ref<OffscreenRenderTarget>& offscreenRenderTarget,
            uint32_t width,
            uint32_t height);

        // scene init = migrate old Application initVulkan "scene part"
        void init();

        // per-frame update: camera movement + uniform updates (migrated from old mainLoop)
        void update(float dt, uint32_t frameIndex);

        // record all command buffers (migrated from old createCommandBuffers)
        void recordCommandBuffers();

        // access command buffer by swapchain image index
        VKFW::Ref<VKFW::vulkancore::CommandBuffer> getCommandBuffer(uint32_t imageIndex) const {
            return mCommandBuffers.at(imageIndex);
        }

        uint32_t getImageCount() const;

        // called when swapchain/offscreen resized/recreated
        void onResize(
            const VKFW::Ref<VKFW::vulkancore::SwapChain>& swapChain,
            const VKFW::Ref<VKFW::vulkancore::RenderPass>& swapChainRenderPass,
            const VKFW::Ref<OffscreenRenderTarget>& offscreenRenderTarget,
            uint32_t width,
            uint32_t height);

        // scene owns a "main camera"
        Camera& getMainCamera() { return mMainCamera; }
        const Camera& getMainCamera() const { return mMainCamera; }

    private:
        // ensure uniform manager has required per-frame UBOs (NVP/Object/Camera) before build()
        void ensureDefaultPerFrameUBOs(const UniformManager::Ptr& um);

        // build pipelines using PipelineFactory
        void buildPipelines();

        // build nodes, materials, uniforms, models, and HDRI resources
        void initResourcesAndNodes();

    private:
        VKFW::Ref<VKFW::vulkancore::Device> mDevice;
        VKFW::Ref<VKFW::vulkancore::CommandPool> mCommandPool;
        VKFW::Ref<VKFW::vulkancore::SwapChain> mSwapChain;
        VKFW::Ref<VKFW::vulkancore::RenderPass> mSwapChainRenderPass;
        VKFW::Ref<OffscreenRenderTarget> mOffscreenRenderTarget;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;

        // factory
        VKFW::Ref<PipelineFactory> mPipelineFactory{ nullptr };

        // scene-level camera
        Camera mMainCamera{};

        // HDRI tools/resources
        VKFW::Ref<HDRITools> mHDRITools{ nullptr };
        VKFW::Ref<VKFW::vulkancore::Image> mHDRICubemap{ nullptr };
        VKFW::Ref<VKFW::vulkancore::Image> mDiffuseIrradianceMap{ nullptr };
        VKFW::Ref<VKFW::vulkancore::Image> mSpecularPrefilterMap{ nullptr };
        VKFW::Ref<VKFW::vulkancore::Image> mBRDFLUT{ nullptr };

        // push constants (kept to match old behavior)
        PushConstantManager::Ptr mPushConstantManager{ nullptr };

        // nodes (all migrated from old sphere/offscreenSphere/skybox)
        VKFW::Ref<RenderNode> mScreenQuadNode{ nullptr };   // sample offscreen RT -> swapchain
        VKFW::Ref<RenderNode> mOffscreenPBRNode{ nullptr }; // render PBR model -> offscreen RT
        VKFW::Ref<RenderNode> mSkyboxNode{ nullptr };       // render skybox -> offscreen RT

        // pipelines
        VKFW::Ref<VKFW::vulkancore::Pipeline> mPBRPipeline{ nullptr };
        VKFW::Ref<VKFW::vulkancore::Pipeline> mScreenQuadPipeline{ nullptr };
        VKFW::Ref<VKFW::vulkancore::Pipeline> mSkyboxPipeline{ nullptr };

        // command buffers (one per swapchain image)
        std::vector<VKFW::Ref<VKFW::vulkancore::CommandBuffer>> mCommandBuffers{};

        // per-frame uniforms reused
        NVPMatrices mNVPMatrices{};
        cameraParameters mCameraParameters{};
    };

} // namespace VKFW::renderer