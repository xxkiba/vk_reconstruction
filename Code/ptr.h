#pragma once
#include <memory>
#include <utility>

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
