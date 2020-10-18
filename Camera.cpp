#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace SoftwareRasterizer
{
	glm::mat4 Camera::getViewMatrix()
	{
		return ViewMatrix;
	}

	void Camera::Update()
	{
		// calc view matrix and related vectors.
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
		ViewMatrix = glm::lookAt(this->position, this->position + this->front, this->up);
	}
}