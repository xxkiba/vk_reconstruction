#include "renderPass.h"

namespace VKFW::vulkancore {

    void SubPass::validate() const {
        if (mColorAttachmentReferences.empty()) {
            throw std::runtime_error("SubPass: color attachment references are empty");
        }

    }

    RenderPass::RenderPass(const VKFW::Ref<Device>& device)
        : mDevice(device) {
    }

    RenderPass::~RenderPass() {
        if (mRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(mDevice->getDevice(), mRenderPass, nullptr);
            mRenderPass = VK_NULL_HANDLE;
        }
        mAttachmentDescriptions.clear();
        mSubPasses.clear();
        mDependencies.clear();
        mDevice.reset();
    }

    void RenderPass::addAttachment(const VkAttachmentDescription& attachmentDescription) {
        mAttachmentDescriptions.push_back(attachmentDescription);
    }

    void RenderPass::addSubpass(const SubPass& subpass) {

        mSubPasses.push_back(subpass);
    }

    void RenderPass::addDependency(const VkSubpassDependency& dependency) {
        mDependencies.push_back(dependency);
    }

    void RenderPass::buildRenderPass() {
        if (mSubPasses.empty() || mAttachmentDescriptions.empty()) {
            throw std::runtime_error("RenderPass: subpasses or attachments are empty");
        }


        for (const auto& sp : mSubPasses) {
            sp.validate();
        }


        std::vector<VkSubpassDescription> subpassDescs(mSubPasses.size());

        for (size_t i = 0; i < mSubPasses.size(); ++i) {
            const auto& sp = mSubPasses[i];
            VkSubpassDescription& desc = subpassDescs[i];
            desc = {};

            desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            desc.inputAttachmentCount = static_cast<uint32_t>(sp.mInputAttachmentReferences.size());
            desc.pInputAttachments = sp.mInputAttachmentReferences.empty() ? nullptr
                : sp.mInputAttachmentReferences.data();

            desc.colorAttachmentCount = static_cast<uint32_t>(sp.mColorAttachmentReferences.size());
            desc.pColorAttachments = sp.mColorAttachmentReferences.data();


            desc.pResolveAttachments = sp.mResolveAttachmentReference ? &(*sp.mResolveAttachmentReference) : nullptr;
            desc.pDepthStencilAttachment = sp.mDepthStencilAttachmentReference ? &(*sp.mDepthStencilAttachmentReference) : nullptr;
        }

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(mAttachmentDescriptions.size());
        renderPassCreateInfo.pAttachments = mAttachmentDescriptions.data();

        renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpassDescs.size());
        renderPassCreateInfo.pSubpasses = subpassDescs.data();

        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(mDependencies.size());
        renderPassCreateInfo.pDependencies = mDependencies.empty() ? nullptr : mDependencies.data();

        if (vkCreateRenderPass(mDevice->getDevice(), &renderPassCreateInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to create render pass!");
        }
    }

} // namespace VKFW::vulkancore