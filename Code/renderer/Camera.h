#pragma once
#include"render_common.h"

enum class CAMERA_MOVE
{
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_FRONT,
	MOVE_BACK
};
class Camera
{
private:
	glm::vec3	m_position;
	glm::vec3	m_front;
	glm::vec3	m_up;

	glm::vec3	mInitialViewDirection;
	glm::vec3	m_targetPosition; // Target position for the camera to look at
	float	mDistanceFromTarget; // Distance from the target position

	float		m_speed;

	float		m_pitch;
	float		m_yaw;
	float		m_sensitivity;

	float		m_xpos;
	float       m_ypos;
	bool		m_firstMove;

	float 	mRotateAngle;
	glm::mat4	m_pMatrx;
	glm::mat4	m_vMatrix;

public:

	Camera()
	{
		m_position = glm::vec3(1.0f);
		m_front = glm::vec3(1.0f);
		m_up = glm::vec3(1.0f);

		mInitialViewDirection = glm::normalize(glm::vec3(0.0f, -0.2f, 1.0f)); // Default view direction looking down the negative Z-axis
		m_targetPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Default target position at the origin
		mDistanceFromTarget = 5.0f; // Default distance from the target position

		m_speed = 0.01f;

		m_pitch = 0.0f;
		m_yaw = -90.0f;
		m_sensitivity = 0.1f;

		m_xpos = 0;
		m_ypos = 0;
		m_firstMove = true;

		m_vMatrix = glm::mat4(1.0f);
		m_pMatrx = glm::mat4(1.0f);

		mRotateAngle = 0.0f;
	}
	~Camera()
	{

	}
	void Init(glm::vec3 inTargetPosition, float inDistanceFromTarget, glm::vec3 inViewDirection);
	void lookAt(glm::vec3 _pos, glm::vec3 _target, glm::vec3 _up);
	void update();
	void horizontalRoundRotate(float inDeltaTime, glm::vec3 inTargetPosition, float inDistanceFromTarget, float inRotateSpeed);
	glm::vec4 getCamPosition();

	glm::mat4 getViewMatrix();

	glm::mat4 getProjectMatrix();

	void	setSpeed(float _speed)
	{
		m_speed = _speed;
	}

	void move(CAMERA_MOVE _mode);

	void pitch(float _yOffset);
	void yaw(float _xOffset);
	void setSentitivity(float _s);
	void onMouseMove(double _xpos, double _ypos);

	void setPerpective(float angle, float ratio, float near, float far);
};

