// RenderNode.h
#pragma once

#include "render_common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// IMPORTANT: GLM_ENABLE_EXPERIMENTAL must be defined BEFORE including gtx headers
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>

#include "model.h"
#include "Camera.h"
#include "uniformManager.h"
#include "Material.h"
#include "../vulkancore/commandBuffer.h"

#include <optional>
#include <string>
#include <vector>

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
        void setRotationQuat(const glm::quat& q);          // optional: more robust than euler
        void setScale(float x, float y, float z);

        // Convenience: set TRS in one call
        void setTRS(const glm::vec3& pos, const glm::vec3& eulerRad, const glm::vec3& scl);

        const glm::mat4& getModelMatrix()  const { return mModelMatrix; }
        const glm::mat4& getNormalMatrix() const { return mNormalMatrix; }

        // Call if you edited transform fields directly
        void markTransformDirty() { mTransformDirty = true; }
        void updateTransformIfNeeded();

        // -------------------------
        // Render metadata (shader/pipeline related)
        // -------------------------
        void setShaderFiles(std::string vs, std::string fs);
        const std::string& getVertexShaderFile()   const { return mVertexShaderFile; }
        const std::string& getFragmentShaderFile() const { return mFragmentShaderFile; }

        // Optional: identify which "pass" / "pipeline family" this node belongs to
        // Example: "gbuffer", "shadow", "forward", "skybox"
        void setRenderTag(std::string tag) { mRenderTag = std::move(tag); }
        const std::string& getRenderTag() const { return mRenderTag; }

        // Vertex input:
        // - If override is set, return override
        // - Else query from first model
        void setVertexInputOverride(
            std::vector<VkVertexInputBindingDescription> bindings,
            std::vector<VkVertexInputAttributeDescription> attrs);

        bool hasVertexInputOverride() const { return mVertexInputOverride.has_value(); }

        std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions() const;
        std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() const;

        // Descriptor set layouts:
        // - Base: UniformManager + Material
        // - Extension: user can add extra layouts (e.g. scene, lights, pass-specific)
        void setExtraDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> extra);
        const std::vector<VkDescriptorSetLayout>& getExtraDescriptorSetLayouts() const { return mExtraLayouts; }

        std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() const;

        // -------------------------
        // Rendering
        // -------------------------
        void draw(const VKFW::Ref<VKFW::vulkancore::CommandBuffer>& cmdBuf);

        // Optional: per-node logic update hook
        virtual void update() {}

        // Optional hook: called right before drawing models
        // Good place to push model matrices into uniform/push constants if you want.
        virtual void beforeDraw(const VKFW::Ref<VKFW::vulkancore::CommandBuffer>& /*cmdBuf*/) {}

    public:
        // -------------------------
        // Public members (minimal refactor)
        // -------------------------
        std::vector<VKFW::Ref<Model>> mModels{};
        VKFW::Ref<Material> mMaterial{ nullptr };
        UniformManager::Ptr mUniformManager{ nullptr };

        // RenderNode owns a camera for this render instance
        Camera mCamera{};

        // Transform fields
        glm::vec4 mPosition{ 0.0f, 0.0f, 0.0f, 1.0f };
        glm::vec4 mRotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // xyz used (Euler rad), w unused
        glm::vec4 mScale{ 1.0f, 1.0f, 1.0f, 1.0f };

    private:
        // Cached matrices
        glm::mat4 mModelMatrix{ 1.0f };
        glm::mat4 mNormalMatrix{ 1.0f };

        bool mTransformDirty{ true };

        // If you choose quat rotation, we store it (optional). If not set, use Euler.
        std::optional<glm::quat> mRotationQuatOverride{ std::nullopt };

        // Shader identity
        std::string mVertexShaderFile{};
        std::string mFragmentShaderFile{};

        // Optional tag to classify pipeline/pass
        std::string mRenderTag{};

        // Vertex input override
        struct VertexInputOverride {
            std::vector<VkVertexInputBindingDescription> bindings;
            std::vector<VkVertexInputAttributeDescription> attrs;
        };
        std::optional<VertexInputOverride> mVertexInputOverride{ std::nullopt };

        // Extra descriptor set layouts appended after base ones
        std::vector<VkDescriptorSetLayout> mExtraLayouts{};
    };

} // namespace VKFW::renderer