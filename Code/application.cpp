// application.cpp
#include "application.h"
#include <limits>

namespace VKFW {

    void Application::run() {
        initWindow();
        initVulkan();
        initScene();
        createSyncObjects();
        mainLoop();
        cleanUp();
    }

    void Application::initWindow() {
        mWindow = MakeRef<platform::Window>(mWidth, mHeight);

        mWindow->setOnResize([this](int w, int h) { this->onResize(w, h); });
        mWindow->setOnMouseMove([this](double x, double y) { this->onMouseMove(x, y); });
        mWindow->setOnKeyMove([this](platform::CameraMove mv) { this->onKeyMove(mv); });
    }

    void Application::initVulkan() {
        mInstance = MakeRef<vulkancore::Instance>(true);
        mSurface = MakeRef<vulkancore::WindowSurface>(mInstance, mWindow);
        mDevice = MakeRef<vulkancore::Device>(mInstance, mSurface);
        mCommandPool = MakeRef<vulkancore::CommandPool>(mDevice);
        mSwapChain = MakeRef<vulkancore::SwapChain>(mDevice, mWindow, mSurface, mCommandPool);

        mRenderPass = factories::RenderPassFactory::CreateMsaaRenderPass(
            mDevice,
            mSwapChain->getSwapChainImageFormat(),
            mDevice->getMaxUsableSampleCount(),
            vulkancore::Image::findDepthFormat(mDevice)
        );

        mSwapChain->createFrameBuffers(mRenderPass);

        mOffscreenRenderTarget = VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(
            mDevice, mCommandPool,
            mWidth, mHeight,
            mSwapChain->getImageCount(),
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT
        );
    }

    void Application::initScene() {
        mScene = VKFW::renderer::Scene::create(
            mDevice,
            mCommandPool,
            mSwapChain,
            mRenderPass,
            mOffscreenRenderTarget,
            (uint32_t)mWidth,
            (uint32_t)mHeight
        );
        mScene->init();
    }

    float Application::getFrameTime() {
        static double lastTime = 0.0;
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        return deltaTime;
    }
    CAMERA_MOVE Application::toCameraMove(platform::CameraMove mv) {
        switch (mv) {
        case platform::CameraMove::Left:  return CAMERA_MOVE::MOVE_LEFT;
        case platform::CameraMove::Right: return CAMERA_MOVE::MOVE_RIGHT;
        case platform::CameraMove::Front: return CAMERA_MOVE::MOVE_FRONT;
        case platform::CameraMove::Back:  return CAMERA_MOVE::MOVE_BACK;
        default:                          return CAMERA_MOVE::MOVE_FRONT;
        }
    }
    void Application::mainLoop() {
        while (!mWindow->shouldClose()) {
            mWindow->pollEvents();
            mWindow->processEvents();

            float dt = getFrameTime();

            // update scene
            if (mScene) {
                mScene->update(dt, mCurrentFrame);
            }

            // render
            render();
        }

        vkDeviceWaitIdle(mDevice->getDevice());
    }

    void Application::createSyncObjects() {
        const uint32_t count = mSwapChain->getImageCount();
        mImageAvailableSemaphores.clear();
        mRenderFinishedSemaphores.clear();
        mFences.clear();

        for (uint32_t i = 0; i < count; i++) {
            mImageAvailableSemaphores.push_back(VKFW::MakeRef<vulkancore::Semaphore>(mDevice));
            mRenderFinishedSemaphores.push_back(VKFW::MakeRef<vulkancore::Semaphore>(mDevice));
            mFences.push_back(VKFW::MakeRef<vulkancore::Fence>(mDevice, true));
        }
    }

    void Application::render() {
        // wait fence
        mFences[mCurrentFrame]->waitForFence();

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(
            mDevice->getDevice(),
            mSwapChain->getSwapChain(),
            std::numeric_limits<uint64_t>::max(),
            mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(),
            VK_NULL_HANDLE,
            &imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
            recreateSwapChain();
            mFramebufferResized = false;
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to acquire swapchain image.");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkCommandBuffer cmd = mScene->getCommandBuffer(imageIndex)->getCommandBuffer();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        mFences[mCurrentFrame]->resetFence();
        if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer.");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { mSwapChain->getSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
            recreateSwapChain();
            mFramebufferResized = false;
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image.");
        }

        mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
    }

    void Application::recreateSwapChain() {

        // Wait until the window size is not zero.
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(mWindow->getWindow(), &fbW, &fbH);
        while (fbW == 0 || fbH == 0) {
            glfwWaitEvents();
            glfwGetFramebufferSize(mWindow->getWindow(), &fbW, &fbH);
        }

        vkDeviceWaitIdle(mDevice->getDevice());

        mImageAvailableSemaphores.clear();
        mRenderFinishedSemaphores.clear();
        mFences.clear();
        mOffscreenRenderTarget.reset();
        mRenderPass.reset();
        mSwapChain.reset();
        mCurrentFrame = 0;
        if (mScene) {
            mScene->detachSwapchainRefs();
        }
        // recreate swapchain
        mSwapChain = MakeRef<vulkancore::SwapChain>(mDevice, mWindow, mSurface, mCommandPool);

		// use swapchain extent as new width and height
        auto extent = mSwapChain->getSwapChainExtent();
        mWidth = (int)extent.width;
        mHeight = (int)extent.height;


        // render pass might be rebuild or reused depending on your factory; simplest rebuild:
        mRenderPass = factories::RenderPassFactory::CreateMsaaRenderPass(
            mDevice,
            mSwapChain->getSwapChainImageFormat(),
            mDevice->getMaxUsableSampleCount(),
            vulkancore::Image::findDepthFormat(mDevice)
        );

        mSwapChain->createFrameBuffers(mRenderPass);

        // recreate offscreen RT
        mOffscreenRenderTarget = VKFW::MakeRef<VKFW::renderer::OffscreenRenderTarget>(
            mDevice, mCommandPool,
            (uint32_t)mWidth, (uint32_t)mHeight,
            mSwapChain->getImageCount(),
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT
        );

        // inform scene to rebuild pipelines/material/command buffers
        if (mScene) {
            mScene->onResize(mSwapChain, mRenderPass, mOffscreenRenderTarget, (uint32_t)mWidth, (uint32_t)mHeight);
        }

        createSyncObjects();
        mCurrentFrame = 0;
    }

    void Application::cleanUp() {
        vkDeviceWaitIdle(mDevice->getDevice());

        mScene.reset();
        mOffscreenRenderTarget.reset();
        mRenderPass.reset();
        mSwapChain.reset();
        mCommandPool.reset();
        mDevice.reset();
        mSurface.reset();
        mInstance.reset();
        mWindow.reset();
    }

    void Application::onResize(int w, int h) {
        mWidth = w;
        mHeight = h;
        mFramebufferResized = true;
    }

    void Application::onMouseMove(double x, double y) {
        if (mScene) {
            if (auto cam = mScene->getMainCamera()) cam->onMouseMove(x, y);
        }
    }

    void Application::onKeyMove(platform::CameraMove move) {
        if (!mScene) return;
        if (auto cam = mScene->getMainCamera()) cam->move(toCameraMove(move));
    }

} // namespace VKFW