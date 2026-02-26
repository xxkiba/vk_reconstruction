#pragma once
#include <memory>
#include <utility>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define GLFW_INCLUDE_VULKAN


#include <glfw/glfw3.h>
namespace VKFW {

    template<class T>
    using Ref = std::shared_ptr<T>;

    template<class T>
    using WeakRef = std::weak_ptr<T>;

    template<class T>
    using Scope = std::unique_ptr<T>;

    template<class T, class... Args>
    [[nodiscard]] inline Ref<T> MakeRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    [[nodiscard]] inline Scope<T> MakeScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

} // namespace VKFW
