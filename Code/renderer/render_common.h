#pragma once

#include "../ptr.h"

struct NVPMatrices {
	glm::mat4 mNormalMatrix{ 1.0f };
	glm::mat4 mViewMatrix{ 1.0f };
	glm::mat4 mProjectionMatrix{ 1.0f };
};

struct ObjectUniform {
	glm::mat4 mModelMatrix{ 1.0f };
	//glm::vec4 mColor{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct cameraParameters {
	glm::vec4 CameraWorldPosition{ 0.0f, 0.0f, 0.0f, 1.0f };
};