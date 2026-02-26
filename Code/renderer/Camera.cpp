#include "Camera.h"

void Camera::lookAt(glm::vec3 _pos, glm::vec3 _target, glm::vec3 _up)
{
	m_position = _pos;
	m_front = glm::normalize(_target - _pos);


	glm::vec3 right = glm::normalize(glm::cross(m_front, _up));
	m_up = glm::normalize(glm::cross(right, m_front));

	m_vMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::update()
{
	m_vMatrix = glm::lookAt(m_position, m_targetPosition, m_up);
}

glm::vec4 Camera::getCamPosition()
{
	return glm::vec4(m_position, 1.0f);
}

glm::mat4 Camera::getViewMatrix()
{
	return m_vMatrix;
}

glm::mat4 Camera::getProjectMatrix() {
	return m_pMatrx;
}

void Camera::move(CAMERA_MOVE _mode)
{
	switch (_mode)
	{
	case CAMERA_MOVE::MOVE_LEFT:
		m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_speed;
		break;
	case CAMERA_MOVE::MOVE_RIGHT:
		m_position += glm::normalize(
			glm::cross(m_front, m_up)) * m_speed;
		break;
	case CAMERA_MOVE::MOVE_FRONT:
		m_position += m_speed * m_front;
		break;
	case CAMERA_MOVE::MOVE_BACK:
		m_position -= m_speed * m_front;
		break;
	default:
		break;
	}

	update();
}

void Camera::pitch(float _yOffset)
{
	m_pitch += _yOffset * m_sensitivity;

	if (m_pitch >= 89.0f)
	{
		m_pitch = 89.0f;
	}

	if (m_pitch <= -89.0f)
	{
		m_pitch = -89.0f;
	}

	m_front.y = sin(glm::radians(m_pitch));
	m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(m_front);
	update();
}
void Camera::yaw(float _xOffset)
{
	m_yaw += _xOffset * m_sensitivity;

	m_front.y = sin(glm::radians(m_pitch));
	m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(m_front);
	update();
}
void Camera::Init(glm::vec3 inTargetPosition, float inDistanceFromTarget, glm::vec3 inViewDirection) {
	m_targetPosition = inTargetPosition;
	inViewDirection = glm::normalize(inViewDirection);
	mInitialViewDirection = inViewDirection;
	mDistanceFromTarget = inDistanceFromTarget;
	mRotateAngle = 0.0f;
	glm::vec3 cameraPosition = inTargetPosition - inViewDirection * inDistanceFromTarget;
	glm::vec3 rightDirection = glm::cross(inViewDirection, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 upDirection = glm::normalize(glm::cross(rightDirection, inViewDirection));
	m_position = cameraPosition;
	m_front = inViewDirection;
	m_up = upDirection;
	update();
}


void Camera::horizontalRoundRotate(float inDeltaTime, glm::vec3 inTargetPosition, float inDistanceFromTarget, float inRotateSpeed) {

	m_targetPosition = inTargetPosition;
	float newAngle = mRotateAngle + inRotateSpeed * inDeltaTime;
	mRotateAngle = newAngle;
	glm::mat4 rotateMatrix;
	rotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(newAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 inverseInitialViewDirection = -mInitialViewDirection;
	glm::vec3 inverseViewDirection = glm::vec3(rotateMatrix * glm::vec4(inverseInitialViewDirection, 0.0f));
	inverseViewDirection = glm::normalize(inverseViewDirection);

	glm::vec3 viewDirection = -inverseViewDirection;

	glm::vec3 mCameraPosition = inTargetPosition - viewDirection * inDistanceFromTarget;
	m_position = mCameraPosition;
	m_front = glm::normalize(viewDirection);
	glm::vec3 rightDirection = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 upDirection = glm::normalize(glm::cross(rightDirection, m_front));
	m_up = upDirection;
	update();
}

void Camera::setSentitivity(float _s)
{
	m_sensitivity = _s;
}

void Camera::setPerpective(float angle, float ratio, float near, float far) {
	m_pMatrx = glm::perspective(glm::radians(angle), ratio, near, far);
}

void Camera::onMouseMove(double _xpos, double _ypos)
{
	//if (m_firstMove)
	//{
	//	m_xpos = _xpos;
	//	m_ypos = _ypos;
	//	m_firstMove = false;
	//	return;
	//}

	//float _xOffset = _xpos - m_xpos;
	//float _yOffset = -(_ypos - m_ypos);

	//m_xpos = _xpos;
	//m_ypos = _ypos;

	//pitch(_yOffset);
	//yaw(_xOffset);
}
