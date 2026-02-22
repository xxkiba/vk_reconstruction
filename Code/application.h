#include "stdHeader.h"
#include "platform/window.h"
#include "vulkancore/instance.h"
#include "vulkancore/windowSurface.h"
#include "vulkancore/device.h"
#include "vulkancore/commandPool.h"
#include "vulkancore/renderPass.h"
#include "vulkancore/image.h"
#include "vulkancore/swapChain.h"

#include "factories/renderPassFactory.h"
#include "ptr.h"
namespace VKFW {

	class Application {
    public:
        void run();


    private:
        void initWindow();
        void initVulkan();
        void mainLoop();
        void cleanUp();

        void onResize(int w, int h);
        void onMouseMove(double x, double y);
        void onKeyMove(platform::CameraMove move);

    private:
        Ref<platform::Window> mWindow{ nullptr };
        Ref<vulkancore::Instance> mInstance{ nullptr };
        Ref<vulkancore::WindowSurface> mSurface{ nullptr };
        Ref<vulkancore::Device> mDevice{ nullptr };
        Ref<vulkancore::CommandPool> mCommandPool{ nullptr };
        Ref<vulkancore::RenderPass> mRenderPass{ nullptr };
        Ref<vulkancore::SwapChain> mSwapChain{ nullptr };

        int mWidth{ 1280 }, mHeight{ 720 };

        // Camera mCamera;
        // Renderer mRenderer;
    };
}