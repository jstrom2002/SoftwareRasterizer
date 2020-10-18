#pragma once
#include <glm/glm.hpp>

namespace SoftwareRasterizer
{
	class Camera
	{
	public:
		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 right;
		glm::vec3 up;
		float movementSpeed;

		Camera() : position(glm::vec3(0)), front(glm::vec3(0,0,-1)), right(glm::vec3(1,0,0)),
			up(glm::vec3(0,1,0)), movementSpeed(0.01), worldUp(glm::vec3(0,1,0)),
			ViewMatrix(glm::mat4(0)) {}
		glm::mat4 getViewMatrix();
		void Update();

	private:
		glm::vec3 worldUp;
		glm::mat4 ViewMatrix;
	};
}