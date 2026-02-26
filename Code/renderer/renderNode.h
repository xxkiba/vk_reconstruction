// RenderNode.h
#pragma once

#include "render_common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model.h"
#include "Camera.h"
#include "uniformManager.h"
#include "Material.h"

#include "../vulkancore/commandBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace VKFW::renderer {

    class RenderNode {
    public:

        RenderNode();
        ~RenderNode() = default;

        // -------------------------
        // Transform
        // -------------------------
        void setPosition(float x, float y, float z);
        void setRotationEuler(float x, float y, float z); // radians
        void setScale(float x, float y, float z);

        const glm::mat4& getModelMatrix()  const { return mModelMatrix; }
        const glm::mat4& getNormalMatrix() const { return mNormalMatrix; }

        // Call if you edited transform fields directly
        void markTransformDirty() { mTransformDirty = true; }
        void updateTransformIfNeeded();

        // -------------------------
        // Render metadata (shader/pipeline related)
        // -------------------------
        void setShaderFiles(std::string vs, std::string fs);
        const std::string& getVertexShaderFile() const { return mVertexShaderFile; }
        const std::string& getFragmentShaderFile() const { return mFragmentShaderFile; }

        // Vertex input (by default from first model)
        std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions() const;
        std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() const;

        // Descriptor set layouts (uniform + material)
        std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() const;

        // -------------------------
        // Rendering
        // -------------------------
        void draw(const VKFW::Ref<VKFW::vulkancore::CommandBuffer>& cmdBuf);

        // Optional: per-node logic update hook
        virtual void update() {}

    public:
        // -------------------------
        // Public members (keep same style as your current code for minimal refactor)
        // -------------------------
        std::vector<VKFW::Ref<Model>> mModels{};
        VKFW::Ref<Material> mMaterial{ nullptr };
        UniformManager::Ptr mUniformManager{ nullptr };
        Camera mCamera{}; // kept for compatibility (can be moved to Scene later)

        // Transform fields (kept similar to your original SceneNode)
        glm::vec4 mPosition{ 0.0f, 0.0f, 0.0f, 1.0f };
        glm::vec4 mRotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // xyz used, w unused
        glm::vec4 mScale{ 1.0f, 1.0f, 1.0f, 1.0f };

    private:
        // Cached matrices
        glm::mat4 mModelMatrix{ 1.0f };
        glm::mat4 mNormalMatrix{ 1.0f };

        bool mTransformDirty{ true };

        // Shader “identity” (lets renderer build pipelines based on node config)
        std::string mVertexShaderFile{};
        std::string mFragmentShaderFile{};
    };

} // namespace VKFW::renderer