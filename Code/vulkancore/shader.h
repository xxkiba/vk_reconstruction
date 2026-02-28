#pragma once


#include "vk_common.h"
#include "device.h"

namespace VKFW::vulkancore {
	class Shader {
	public:
		Shader(const VKFW::Ref<Device>& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint);
		~Shader();

		VkShaderModule getShaderModule() const { return mShaderModule; }
		[[nodiscard]] const std::string& getEntryPoint() const { return mEntryPoint; }
		[[nodiscard]] VkShaderStageFlagBits getShaderStage() const { return mShaderStage; }

	private:
		void createShaderModule(const std::string& fileName);
		// Read the binary file and return the contents as a vector of chars
		static std::vector<char> readBinary(const std::string& fileName) {
			std::ifstream file(fileName.c_str(), std::ios::ate | std::ios::binary | std::ios::in);
			if (!file.is_open()) {
				throw std::runtime_error("failed to open shader file!");
			}
			const size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);
			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}
	private:
		VKFW::Ref<Device> mDevice{ nullptr };
		VkShaderModule mShaderModule{ VK_NULL_HANDLE };
		std::string mEntryPoint{ "main" };
		VkShaderStageFlagBits mShaderStage{ VK_SHADER_STAGE_VERTEX_BIT };
	};
}