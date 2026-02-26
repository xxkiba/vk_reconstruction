#pragma once
#include "../ptr.h"
#include "vk_common.h"

namespace VKFW::vulkancore {
	class ConstantRange {
	public:
		ConstantRange(uint32_t offset, uint32_t size, uint32_t stageFlags);
		~ConstantRange();
		[[nodiscard]] auto getPushConstantRange() const {
			return mPushConstantRange;
		}
		void updateConstantRange(uint32_t offset, uint32_t size, uint32_t stageFlags) {
			mPushConstantRange.offset = offset;
			mPushConstantRange.size = size;
			mPushConstantRange.stageFlags = stageFlags;
		}

	private:
		VkPushConstantRange mPushConstantRange{};
	};
} //namespace VKFW::vulkancore