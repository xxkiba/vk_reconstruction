#pragma once
#include "../vulkancore/dataBuffer.h"
#include "../vulkancore/descriptorSetLayout.h"
#include "../vulkancore/descriptorPool.h"
#include "../vulkancore/description.h"
#include "../vulkancore/descriptorSet.h"
#include "../vulkancore/device.h"
#include "../vulkancore/commandPool.h"
#include "render_common.h"

namespace VKFW::renderer {
	class UniformManager {
	public:
		using Ptr = std::shared_ptr<UniformManager>;
		static Ptr create() {
			return std::make_shared<UniformManager>();
		}
		UniformManager();
		~UniformManager();
		void init(const VKFW::Ref<VKFW::vulkancore::Device>& device, const VKFW::Ref<VKFW::vulkancore::CommandPool>& commandPool, int frameCount);
		void build();
		void attachCubeMap(VKFW::Ref<VKFW::vulkancore::Image>& inImage);
		void attachImage(VKFW::Ref<VKFW::vulkancore::Image>& inImage);
		void attachMapImage(VKFW::Ref<VKFW::vulkancore::Image>& inImage);
		void updateUniformBuffer(const NVPMatrices& vpMatrices, const ObjectUniform& objectUniform, const cameraParameters& cameraParams, const int frameCount);

		[[nodiscard]] auto getDescriptorLayout() const {
			return mDescriptorLayout;
		}
		[[nodiscard]] auto getDescriptorPool() const {
			return mDescriptorPool;
		}
		[[nodiscard]] auto getDescriptorSet(int frameCount) const {
			return mDescriptorSet->getDescriptorSet(frameCount);
		}
		[[nodiscard]] auto getUniformParameters() const {
			return mUniformParameters;
		}

	private:
		VKFW::Ref<VKFW::vulkancore::Device> mDevice{ nullptr };
		VKFW::Ref<VKFW::vulkancore::CommandPool> mcommandpool{ nullptr };
		std::vector<VKFW::Ref<VKFW::vulkancore::UniformParameter>> mUniformParameters{};

		VKFW::vulkancore::DescriptorSetLayout::Ptr mDescriptorLayout{ nullptr };
		VKFW::vulkancore::DescriptorPool::Ptr mDescriptorPool{ nullptr };
		VKFW::vulkancore::DescriptorSet::Ptr mDescriptorSet{ nullptr };
		int mFrameCount = 1;


	};
}

