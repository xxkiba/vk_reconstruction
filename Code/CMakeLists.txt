cmake_minimum_required(VERSION 3.20)
project(VulkanFramework)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. SRC_LIST)

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/GLM/include
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/GLFW/include
	SYSTEM ${VULKAN_SDK_PATH}/include
)

link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/GLFW/lib
	SYSTEM ${VULKAN_SDK_PATH}/Lib
)

add_subdirectory(platform)
add_subdirectory(vulkancore)
add_subdirectory(factories)
add_subdirectory(renderer)

add_executable(VulkanFramework ${SRC_LIST})

target_link_libraries(VulkanFramework glfw3.lib vulkan-1.lib platformLib vulkanLib factoriesLib renderLib)