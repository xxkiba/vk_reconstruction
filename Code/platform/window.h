#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN


#include <glfw/glfw3.h>
#include <functional>
#include <optional>
#include "inputTypes.h" 

namespace VKFW::platform {

    class Window {
    public:
        using ResizeFn = std::function<void(int, int)>;
        using MouseMoveFn = std::function<void(double, double)>;
        using KeyMoveFn = std::function<void(CameraMove)>;

        Window(int width, int height);
        ~Window();

        bool shouldClose() const;
        void pollEvents();
        void processEvents();

        void setOnResize(ResizeFn fn) { mOnResize = std::move(fn); }
        void setOnMouseMove(MouseMoveFn fn) { mOnMouseMove = std::move(fn); }
        void setOnKeyMove(KeyMoveFn fn) { mOnKeyMove = std::move(fn); }

        GLFWwindow* nativeHandle() const { return mWindow; }
        int width()  const { return mWidth; }
        int height() const { return mHeight; }

        [[nodiscard]] auto getWidth() const { return mWidth; }
        [[nodiscard]] auto getHeight() const { return mHeight; }
        [[nodiscard]] auto getWindow() const { return mWindow; }

    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

        GLFWwindow* mWindow{ nullptr };
        int mWidth{ 0 }, mHeight{ 0 };

        bool mWindowResized{ false };

        ResizeFn mOnResize;
        MouseMoveFn mOnMouseMove;
        KeyMoveFn mOnKeyMove;
    };

} // namespace