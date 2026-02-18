#include "window.h"
#include <iostream>

namespace VKFW::platform {

    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        self->mWindowResized = true;
        self->mWidth = width;
        self->mHeight = height;

        if (self->mOnResize) self->mOnResize(width, height);
    }

    void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        if (self->mOnMouseMove) self->mOnMouseMove(xpos, ypos);
    }

    Window::Window(int width, int height) : mWidth(width), mHeight(height) {
        if (!glfwInit()) {
            std::cerr << "Error: glfwInit failed\n";
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        mWindow = glfwCreateWindow(mWidth, mHeight, "vulkan window", nullptr, nullptr);
        if (!mWindow) {
            std::cerr << "Error: failed to create window\n";
            glfwTerminate();
            return;
        }

        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
        glfwSetCursorPosCallback(mWindow, cursorPosCallback);
    }

    Window::~Window() {
        if (mWindow) glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    bool Window::shouldClose() const {
        return mWindow && glfwWindowShouldClose(mWindow);
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::processEvents() {
        if (!mWindow) return;

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(mWindow, true);
        }

        if (!mOnKeyMove) return;

        if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) mOnKeyMove(CameraMove::Front);
        if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) mOnKeyMove(CameraMove::Back);
        if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) mOnKeyMove(CameraMove::Left);
        if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) mOnKeyMove(CameraMove::Right);
    }

} // namespace VKFW::platform
