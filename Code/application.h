#include "stdHeader.h"
#include "platform/window.h"
namespace VKFW {

	class Application {
    public:
        void run();

    private:
        void initWindow();
        void mainLoop();

        void onResize(int w, int h);
        void onMouseMove(double x, double y);
        void onKeyMove(platform::CameraMove move);

    private:
        std::unique_ptr<platform::Window> mWindow;
        int mWidth{ 1280 }, mHeight{ 720 };

        // Camera mCamera;
        // Renderer mRenderer;
    };
}