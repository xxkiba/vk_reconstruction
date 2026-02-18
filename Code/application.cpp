#include "application.h"

namespace VKFW {
    void Application::run() {
        initWindow();
        // initVulkan();
        mainLoop();
        // cleanup();
    }

    void Application::initWindow() {
        mWindow = std::make_unique<platform::Window>(mWidth, mHeight);

        mWindow->setOnResize([this](int w, int h) { this->onResize(w, h); });
        mWindow->setOnMouseMove([this](double x, double y) { this->onMouseMove(x, y); });
        mWindow->setOnKeyMove([this](platform::CameraMove mv) { this->onKeyMove(mv); });
    }

    void Application::mainLoop() {
        while (!mWindow->shouldClose()) {
            mWindow->pollEvents();
            mWindow->processEvents();

            // update(dt)
            // render()
        }
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