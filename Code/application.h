#pragma once

#include "stdHeader.h"
#include "ptr.h"

#include "platform/window.h"
#include "platform/inputTypes.h"   // for VKFW::platform::CameraMove

#include "vulkancore/instance.h"
#include "vulkancore/windowSurface.h"
#include "vulkancore/device.h"
#include "vulkancore/commandPool.h"
#include "vulkancore/renderPass.h"
#include "vulkancore/image.h"
#include "vulkancore/swapChain.h"
#include "vulkancore/commandBuffer.h"

#include "vulkancore/semaphore.h"
#include "vulkancore/fence.h"

#include "factories/renderPassFactory.h"
#include "renderer/offscreenRenderTarget.h"

// your camera enum lives here
#include "renderer/Camera.h"

// scene
#include "renderer/scene.h"

namespace VKFW {

    class Application {
    public:
        void run();

    private:
        void initWindow();
        void initVulkan();
        void initScene();

        void createSyncObjects();
        void mainLoop();
        void render();

        void recreateSwapChain();
        void cleanUp();

        float getFrameTime();

        void onResize(int w, int h);
        void onMouseMove(double x, double y);
        void onKeyMove(platform::CameraMove move); // <-- matches Window

    private:
        static CAMERA_MOVE toCameraMove(platform::CameraMove mv);

    private:
        Ref<platform::Window> mWindow{ nullptr };
        Ref<vulkancore::Instance> mInstance{ nullptr };
        Ref<vulkancore::WindowSurface> mSurface{ nullptr };
        Ref<vulkancore::Device> mDevice{ nullptr };
        Ref<vulkancore::CommandPool> mCommandPool{ nullptr };
        Ref<vulkancore::RenderPass> mRenderPass{ nullptr };
        Ref<vulkancore::SwapChain> mSwapChain{ nullptr };

        Ref<renderer::OffscreenRenderTarget> mOffscreenRenderTarget{ nullptr };
        Ref<renderer::Scene> mScene{ nullptr };

        std::vector<Ref<vulkancore::Semaphore>> mImageAvailableSemaphores{};
        std::vector<Ref<vulkancore::Semaphore>> mRenderFinishedSemaphores{};
        std::vector<Ref<vulkancore::Fence>> mFences{};

        uint32_t mCurrentFrame{ 0 };

        int mWidth{ 1280 }, mHeight{ 720 };

        //local resize flag (we don't need Window to expose its private flag)
        bool mFramebufferResized{ false };
    };

} // namespace VKFW