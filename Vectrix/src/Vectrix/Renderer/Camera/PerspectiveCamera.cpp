#include "vcpch.h"

#include "PerspectiveCamera.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "Vectrix/Application.h"

#define FOVY glm::radians(50.f)
#define CAM_NEAR 0.1f
#define CAM_FAR 10.0f

namespace Vectrix {

	PerspectiveCamera::PerspectiveCamera() : m_ViewMatrix(1.0f)
	{
		recalculateMatrices();
	}

	void PerspectiveCamera::recalculateMatrices() {
		recalculateProjectionMatrix();
		recalculateViewMatrix();
		recalculateTransformationMatrix();
	}

	void PerspectiveCamera::recalculateViewMatrix()
	{
		const float c1 = glm::cos(m_Rotation.y); // yaw
		const float s1 = glm::sin(m_Rotation.y);
		const float c2 = glm::cos(m_Rotation.x); // pitch
		const float s2 = glm::sin(m_Rotation.x);
		const float c3 = glm::cos(m_Rotation.z); // roll
		const float s3 = glm::sin(m_Rotation.z);

		const glm::vec3 u{
			c1 * c3 + s1 * s2 * s3,
			c2 * s3,
			c1 * s2 * s3 - c3 * s1
		};

		const glm::vec3 v{
			c3 * s1 * s2 - c1 * s3,
			c2 * c3,
			c1 * c3 * s2 + s1 * s3
		};

		const glm::vec3 w{
			c2 * s1,
		   -s2,
			c1 * c2
		};

		m_ViewMatrix = glm::mat4{1.0f};
		m_ViewMatrix[0][0] = u.x;
		m_ViewMatrix[1][0] = u.y;
		m_ViewMatrix[2][0] = u.z;
		m_ViewMatrix[0][1] = v.x;
		m_ViewMatrix[1][1] = v.y;
		m_ViewMatrix[2][1] = v.z;
		m_ViewMatrix[0][2] = w.x;
		m_ViewMatrix[1][2] = w.y;
		m_ViewMatrix[2][2] = w.z;
		m_ViewMatrix[3][0] = -glm::dot(u, m_Position);
		m_ViewMatrix[3][1] = -glm::dot(v, m_Position);
		m_ViewMatrix[3][2] = -glm::dot(w, m_Position);
	}


	void PerspectiveCamera::recalculateTransformationMatrix() {
		m_transformationMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::recalculateProjectionMatrix() {
		float aspect = VulkanContext::instance().getRenderer().getAspectRatio();
		VC_CORE_ASSERT(aspect > std::numeric_limits<float>::epsilon(),"Aspect ratio is zero or invalid");

		const float tanHalfFovy = std::tan(FOVY / 2.f);
		m_ProjectionMatrix = glm::mat4{0.0f};
		m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_ProjectionMatrix[2][2] = CAM_FAR / (CAM_FAR - CAM_NEAR);
		m_ProjectionMatrix[2][3] = 1.f;
		m_ProjectionMatrix[3][2] = -(CAM_FAR * CAM_NEAR) / (CAM_FAR - CAM_NEAR);

		m_ProjectionMatrix[1][1] *= -1;
	}

	void PerspectiveCamera::setViewDirection(glm::vec3 position, glm::vec3 direction) {
		const glm::vec3 up = {0.0f, 1.0f, 0.0f};
		const glm::vec3 w{glm::normalize(direction)};
		const glm::vec3 u{glm::normalize(glm::cross(w, up))};
		const glm::vec3 v{glm::cross(w, u)};

		m_ViewMatrix = glm::mat4{1.f};
		m_ViewMatrix[0][0] = u.x;
		m_ViewMatrix[1][0] = u.y;
		m_ViewMatrix[2][0] = u.z;
		m_ViewMatrix[0][1] = v.x;
		m_ViewMatrix[1][1] = v.y;
		m_ViewMatrix[2][1] = v.z;
		m_ViewMatrix[0][2] = w.x;
		m_ViewMatrix[1][2] = w.y;
		m_ViewMatrix[2][2] = w.z;
		m_ViewMatrix[3][0] = -glm::dot(u, position);
		m_ViewMatrix[3][1] = -glm::dot(v, position);
		m_ViewMatrix[3][2] = -glm::dot(w, position);

		recalculateTransformationMatrix();
	}

	void PerspectiveCamera::setViewTarget(glm::vec3 position, glm::vec3 target) {
		setViewDirection(position, target - position);
	}

	void PerspectiveCamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
		const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
		const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
		m_ViewMatrix = glm::mat4{1.f};
		m_ViewMatrix[0][0] = u.x;
		m_ViewMatrix[1][0] = u.y;
		m_ViewMatrix[2][0] = u.z;
		m_ViewMatrix[0][1] = v.x;
		m_ViewMatrix[1][1] = v.y;
		m_ViewMatrix[2][1] = v.z;
		m_ViewMatrix[0][2] = w.x;
		m_ViewMatrix[1][2] = w.y;
		m_ViewMatrix[2][2] = w.z;
		m_ViewMatrix[3][0] = -glm::dot(u, position);
		m_ViewMatrix[3][1] = -glm::dot(v, position);
		m_ViewMatrix[3][2] = -glm::dot(w, position);

		recalculateTransformationMatrix();
	}
}
