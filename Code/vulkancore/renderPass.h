#pragma once

#include "../ptr.h"
#include "vk_common.h"
#include "device.h"


namespace VKFW::vulkancore {

    class SubPass {
    public:
        SubPass() = default;
        ~SubPass() = default;

        void addInputAttachmentReference(const VkAttachmentReference& ref) {
            mInputAttachmentReferences.push_back(ref);
        }

        void addColorAttachmentReference(const VkAttachmentReference& ref) {
            mColorAttachmentReferences.push_back(ref);
        }

        void setDepthStencilAttachmentReference(const VkAttachmentReference& ref) {
            mDepthStencilAttachmentReference = ref;
        }

        void setResolveAttachmentReference(const VkAttachmentReference& ref) {
            mResolveAttachmentReference = ref;
        }

        void validate() const;

    private:
        friend class RenderPass;

        std::vector<VkAttachmentReference> mInputAttachmentReferences{};
        std::vector<VkAttachmentReference> mColorAttachmentReferences{};
        std::optional<VkAttachmentReference> mDepthStencilAttachmentReference{};
        std::optional<VkAttachmentReference> mResolveAttachmentReference{};
    };

    class RenderPass {
    public:

        explicit RenderPass(const VKFW::Ref<Device>& device);
        ~RenderPass();

        void addAttachment(const VkAttachmentDescription& attachmentDescription);
        void addSubpass(const SubPass& subpass);
        void addDependency(const VkSubpassDependency& dependency);

        void buildRenderPass();

        VkRenderPass getRenderPass() const { return mRenderPass; }

    private:
        VKFW::Ref<Device> mDevice;
        VkRenderPass mRenderPass{ VK_NULL_HANDLE };

        std::vector<VkAttachmentDescription> mAttachmentDescriptions{};
        std::vector<SubPass> mSubPasses{};
        std::vector<VkSubpassDependency> mDependencies{};
    };

} // namespace VKFW::vulkancore