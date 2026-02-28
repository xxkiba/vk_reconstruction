#include "pushConstantManager.h"

namespace VKFW::renderer {
	PushConstantManager::PushConstantManager() {
	}
	PushConstantManager::~PushConstantManager() {
	}
	void PushConstantManager::init() {

		constantParam vpMatricesParam;
		vpMatricesParam.offset = 0;
		vpMatricesParam.size = sizeof(NVPMatrices);
		vpMatricesParam.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		mConstantParams = vpMatricesParam;
		mPushConstantRange = VKFW::MakeRef < VKFW::vulkancore::ConstantRange>(vpMatricesParam.offset, vpMatricesParam.size, vpMatricesParam.stageFlags);

		mConstantData.offsets[0] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
		mConstantData.offsets[1] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		mConstantData.offsets[2] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	}

	constantData& PushConstantManager::getConstantData() {
		return mConstantData;
	}

	const VKFW::Ref<VKFW::vulkancore::ConstantRange>& PushConstantManager::getPushConstantRanges() {
		return mPushConstantRange;
	}
	const constantParam& PushConstantManager::getConstantParam() const {
		return mConstantParams;
	}
}