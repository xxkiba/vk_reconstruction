#include "stdHeader.h"
#include "platform/window.h"
#include "vulkancore/instance.h"
#include "vulkancore/windowSurface.h"
#include "vulkancore/device.h"
#include "vulkancore/commandPool.h"
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
        Ref<platform::Window> mWindow;
        Ref<vulkancore::Instance> mInstance;
        Ref<vulkancore::WindowSurface> mSurface;
        Ref<vulkancore::Device> mDevice;
        Ref<vulkancore::CommandPool> mCommandPool;

        int mWidth{ 1280 }, mHeight{ 720 };

        // Camera mCamera;
        // Renderer mRenderer;
    };
}