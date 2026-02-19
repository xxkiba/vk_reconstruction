#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <string>
#include <fstream>
#include <optional>
#include <unordered_map>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define GLFW_INCLUDE_VULKAN


#include <glfw/glfw3.h>

inline constexpr std::array<const char*, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

inline constexpr std::array<const char*, 3> deviceRequiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE_1_EXTENSION_NAME,
	VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME
};