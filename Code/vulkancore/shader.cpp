#include "shader.h"

namespace VKFW::vulkancore {


	void Shader::createShaderModule(const std::string& fileName) {
		std::vector<char> code = readBinary(fileName);
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		if (vkCreateShaderModule(mDevice->getDevice(), &shaderModuleCreateInfo, nullptr, &mShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}


	}
	Shader::Shader(const VKFW::Ref<Device>& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint)
		: mDevice(device), mEntryPoint(entryPoint), mShaderStage(shaderStage) {
		createShaderModule(fileName);
	}
	Shader::~Shader() {
		if (mShaderModule != VK_NULL_HANDLE) {
			vkDestroyShaderModule(mDevice->getDevice(), mShaderModule, nullptr);
			mShaderModule = VK_NULL_HANDLE;
		}
		mDevice.reset();
		mEntryPoint.clear();

	}
}