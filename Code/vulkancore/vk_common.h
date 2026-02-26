#pragma once
#include "../ptr.h"
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

inline constexpr std::array<const char*, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

inline constexpr std::array<const char*, 3> deviceRequiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE_1_EXTENSION_NAME,
	VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME
};