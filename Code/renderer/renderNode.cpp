// RenderNode.cpp
#include "RenderNode.h"

#include <stdexcept>
#include <utility>

namespace VKFW::renderer {

    RenderNode::RenderNode() {
        // Keep old code working
        mUniformManager = UniformManager::create();
        mMaterial = Material::create();
        //mCamera = Camera();

        mTransformDirty = true;
    }

    void RenderNode::setPosition(float x, float y, float z) {
        mPosition = glm::vec4(x, y, z, 1.0f);
        mTransformDirty = true;
    }

    void RenderNode::setRotationEuler(float x, float y, float z) {
        mRotation = glm::vec4(x, y, z, 1.0f);
        mRotationQuatOverride.reset(); // Euler becomes the source of truth
        mTransformDirty = true;
    }

    void RenderNode::setRotationQuat(const glm::quat& q) {
        mRotationQuatOverride = q;
        mTransformDirty = true;
    }

    void RenderNode::setScale(float x, float y, float z) {
        mScale = glm::vec4(x, y, z, 1.0f);
        mTransformDirty = true;
    }

    void RenderNode::setTRS(const glm::vec3& pos, const glm::vec3& eulerRad, const glm::vec3& scl) {
        mPosition = glm::vec4(pos, 1.0f);
        mRotation = glm::vec4(eulerRad, 1.0f);
        mRotationQuatOverride.reset();
        mScale = glm::vec4(scl, 1.0f);
        mTransformDirty = true;
    }

    void RenderNode::updateTransformIfNeeded() {
        if (!mTransformDirty) return;

        const glm::vec3 position(mPosition);
        const glm::vec3 scale(mScale);

        glm::quat q;
        if (mRotationQuatOverride.has_value()) {
            q = *mRotationQuatOverride;
        }
        else {
            const glm::vec3 euler(mRotation); // radians
            q = glm::quat(euler);
        }

        const glm::mat4 model =
            glm::translate(glm::mat4(1.0f), position) *
            glm::toMat4(q) *
            glm::scale(glm::mat4(1.0f), scale);

        mModelMatrix = model;

        // Normal matrix: transpose(inverse(mat3(model)))
        const glm::mat3 n3 = glm::transpose(glm::inverse(glm::mat3(model)));
        mNormalMatrix = glm::mat4(1.0f);
        mNormalMatrix[0] = glm::vec4(n3[0], 0.0f);
        mNormalMatrix[1] = glm::vec4(n3[1], 0.0f);
        mNormalMatrix[2] = glm::vec4(n3[2], 0.0f);

        mTransformDirty = false;
    }

    void RenderNode::setShaderFiles(std::string vs, std::string fs) {
        mVertexShaderFile = std::move(vs);
        mFragmentShaderFile = std::move(fs);
    }

    void RenderNode::setVertexInputOverride(
        std::vector<VkVertexInputBindingDescription> bindings,
        std::vector<VkVertexInputAttributeDescription> attrs) {

        VertexInputOverride ov;
        ov.bindings = std::move(bindings);
        ov.attrs = std::move(attrs);
        mVertexInputOverride = std::move(ov);
    }

    std::vector<VkVertexInputBindingDescription> RenderNode::getVertexInputBindingDescriptions() const {
        if (mVertexInputOverride.has_value()) {
            return mVertexInputOverride->bindings;
        }
        if (mModels.empty() || !mModels[0]) {
            // Return empty rather than throw -> allows "create node first, bind model later"
            return {};
        }
        return mModels[0]->getVertexInputBindingDescriptions();
    }

    std::vector<VkVertexInputAttributeDescription> RenderNode::getVertexInputAttributeDescriptions() const {
        if (mVertexInputOverride.has_value()) {
            return mVertexInputOverride->attrs;
        }
        if (mModels.empty() || !mModels[0]) {
            return {};
        }
        return mModels[0]->getAttributeDescriptions();
    }

    void RenderNode::setExtraDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> extra) {
        mExtraLayouts = std::move(extra);
    }

    std::vector<VkDescriptorSetLayout> RenderNode::getDescriptorSetLayouts() const {
        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(2 + mExtraLayouts.size());

        // Base order: (0) per-node uniform, (1) material, (2+) extras
        if (mUniformManager && mUniformManager->getDescriptorLayout()) {
            layouts.push_back(mUniformManager->getDescriptorLayout()->getLayout());
        }
        if (mMaterial && mMaterial->getDescriptorLayout()) {
            layouts.push_back(mMaterial->getDescriptorLayout()->getLayout());
        }
        for (auto l : mExtraLayouts) {
            if (l != VK_NULL_HANDLE) layouts.push_back(l);
        }
        return layouts;
    }

    void RenderNode::draw(const VKFW::Ref<VKFW::vulkancore::CommandBuffer>& cmdBuf) {
        if (!cmdBuf) {
            throw std::runtime_error("RenderNode::draw: cmdBuf is null.");
        }

        updateTransformIfNeeded();
        update();              // optional per-node update hook
        beforeDraw(cmdBuf);    // <--- NEW: let you push matrices/uniforms here

        for (const auto& model : mModels) {
            if (model) {
                model->draw(cmdBuf);
            }
        }
    }

} // namespace VKFW::renderer