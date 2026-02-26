// RenderNode.cpp
#include "RenderNode.h"

#include <stdexcept>

namespace VKFW::renderer {

    RenderNode::RenderNode() {
        // Minimal: create defaults so old code keeps working
        mUniformManager = UniformManager::create();
        mMaterial = Material::create();
        mCamera = Camera();

        mTransformDirty = true;
    }

    void RenderNode::setPosition(float x, float y, float z) {
        mPosition = glm::vec4(x, y, z, 1.0f);
        mTransformDirty = true;
    }

    void RenderNode::setRotationEuler(float x, float y, float z) {
        mRotation = glm::vec4(x, y, z, 1.0f);
        mTransformDirty = true;
    }

    void RenderNode::setScale(float x, float y, float z) {
        mScale = glm::vec4(x, y, z, 1.0f);
        mTransformDirty = true;
    }

    void RenderNode::updateTransformIfNeeded() {
        if (!mTransformDirty) return;

        const glm::vec3 position(mPosition);
        const glm::vec3 scale(mScale);
        const glm::vec3 euler(mRotation); // radians

        const glm::quat q = glm::quat(euler);

        const glm::mat4 model =
            glm::translate(glm::mat4(1.0f), position) *
            glm::toMat4(q) *
            glm::scale(glm::mat4(1.0f), scale);

        mModelMatrix = model;

        // Normal matrix: transpose(inverse(mat3(model)))
        const glm::mat3 m3(model);
        mNormalMatrix = glm::mat4(glm::transpose(glm::inverse(m3)));

        mTransformDirty = false;
    }

    void RenderNode::setShaderFiles(std::string vs, std::string fs) {
        mVertexShaderFile = std::move(vs);
        mFragmentShaderFile = std::move(fs);
    }

    std::vector<VkVertexInputBindingDescription> RenderNode::getVertexInputBindingDescriptions() const {
        if (mModels.empty() || !mModels[0]) {
            throw std::runtime_error("RenderNode: no model to query vertex input bindings.");
        }
        return mModels[0]->getVertexInputBindingDescriptions();
    }

    std::vector<VkVertexInputAttributeDescription> RenderNode::getVertexInputAttributeDescriptions() const {
        if (mModels.empty() || !mModels[0]) {
            throw std::runtime_error("RenderNode: no model to query vertex input attributes.");
        }
        return mModels[0]->getAttributeDescriptions();
    }

    std::vector<VkDescriptorSetLayout> RenderNode::getDescriptorSetLayouts() const {
        std::vector<VkDescriptorSetLayout> layouts;

        if (mUniformManager && mUniformManager->getDescriptorLayout()) {
            layouts.push_back(mUniformManager->getDescriptorLayout()->getLayout());
        }
        if (mMaterial && mMaterial->getDescriptorLayout()) {
            layouts.push_back(mMaterial->getDescriptorLayout()->getLayout());
        }
        return layouts;
    }

    void RenderNode::draw(const VKFW::Ref<VKFW::vulkancore::CommandBuffer>& cmdBuf) {
        if (!cmdBuf) {
            throw std::runtime_error("RenderNode::draw: cmdBuf is null.");
        }

        updateTransformIfNeeded();
        update(); // optional per-node hook

        for (const auto& model : mModels) {
            if (model) {
                model->draw(cmdBuf);
            }
        }
    }

} // namespace VKFW::renderer