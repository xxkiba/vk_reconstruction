#pragma once
#include "render_common.h"
#include "../vulkancore/constantRange.h"

namespace VKFW::renderer {

	struct constantParam {
		uint32_t offset{ 0 };
		uint32_t size{ 0 };
		VkShaderStageFlagBits stageFlags{ VK_SHADER_STAGE_VERTEX_BIT };
	};

	struct constantData {
		glm::vec4 offsets[3]{}; // For future use, currently not used
	};

	class PushConstantManager {
	public:
		using Ptr = std::shared_ptr<PushConstantManager>;
		static Ptr create() {
			return std::make_shared<PushConstantManager>();
		}
		PushConstantManager();
		~PushConstantManager();
		void init();
		const VKFW::Ref<VKFW::vulkancore::ConstantRange>& getPushConstantRanges();
		const constantParam& getConstantParam() const;
		void updateConstantData(const glm::vec4 offsets0, const glm::vec4 offsets1 = glm::vec4(0.0f), const glm::vec4 offsets3 = glm::vec4(0.0f)) {
			mConstantData.offsets[0] = offsets0;
			mConstantData.offsets[1] = offsets1;
			mConstantData.offsets[2] = offsets3;
		}
		void setConstantStageFlags(VkShaderStageFlagBits stageFlags) {
			mConstantParams.stageFlags = stageFlags;
			mPushConstantRange->updateConstantRange(
				mConstantParams.offset,
				mConstantParams.size,
				mConstantParams.stageFlags
			);
		}
		constantData& getConstantData();

	private:
		VKFW::Ref<VKFW::vulkancore::ConstantRange> mPushConstantRange{};
		constantParam mConstantParams{};
		constantData mConstantData{};
	};
}