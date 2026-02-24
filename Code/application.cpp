#include "application.h"

namespace VKFW {
    void Application::run() {
        initWindow();
        initVulkan();
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
        mSurface = MakeRef<vulkancore::WindowSurface>(mInstance,mWindow);
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
            VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
            VK_FORMAT_D24_UNORM_S8_UINT // Depth format
        );
    }

    void Application::mainLoop() {
        while (!mWindow->shouldClose()) {
            mWindow->pollEvents();
            mWindow->processEvents();

            // update(dt)
            // render()
        }
    }

    void Application::cleanUp() {
        vkDeviceWaitIdle(mDevice->getDevice());
        mRenderPass.reset();
        mSwapChain.reset();
        mCommandPool.reset();
        mDevice.reset();
        mSurface.reset();
        mInstance.reset();
        mWindow.reset();
    }
    void Application::onResize(int w, int h) {
        mWidth = w; mHeight = h;

        // renderer.onResize(w, h);
    }

    void Application::onMouseMove(double x, double y) {
        // camera.onMouseMove(x,y);
    }

    void Application::onKeyMove(platform::CameraMove move) {
        // camera.move(move);
    }
}