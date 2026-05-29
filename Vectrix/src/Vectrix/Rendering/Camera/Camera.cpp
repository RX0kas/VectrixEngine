#include "vcpch.h"

#include "Camera.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Framebuffer.h"
#include "Vectrix/Rendering/RendererAPI.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
	Camera* Camera::s_currentCamera = nullptr;

	Camera::Camera(Entity* entity, float fov,float camNear,float camFar) : m_viewMatrix(1.0f),m_fov(glm::radians(fov)),m_camFar(camFar),m_camNear(camNear), m_transform(entity->getComponent<TransformComponent>()) {
		recalculateMatrices();
		setAsCurrent();
	}

	float Camera::getAspect() const {
		return m_customAspect!=-1 ? m_customAspect : Application::instance().window().getAspect();
	}

	void Camera::recalculateMatrices() {
		VC_PROFILER_FUNCTION();
		recalculateProjectionMatrix();
		recalculateViewMatrix();
		recalculateTransformationMatrix();
	}

	void Camera::recalculateViewMatrix()	{
		VC_PROFILER_FUNCTION();
		const float c1 = glm::cos(m_transform.rotation.y); // yaw
		const float s1 = glm::sin(m_transform.rotation.y);
		const float c2 = glm::cos(m_transform.rotation.x); // pitch
		const float s2 = glm::sin(m_transform.rotation.x);
		const float c3 = glm::cos(m_transform.rotation.z); // roll
		const float s3 = glm::sin(m_transform.rotation.z);

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

		m_viewMatrix = glm::mat4{1.0f};
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, m_transform.position);
		m_viewMatrix[3][1] = -glm::dot(v, m_transform.position);
		m_viewMatrix[3][2] = -glm::dot(w, m_transform.position);
	}


	void Camera::recalculateTransformationMatrix() {
		m_transformationMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void Camera::recalculateProjectionMatrix() {
		VC_PROFILER_FUNCTION();
		float aspect = getAspect();
		VC_CORE_ASSERT(aspect > std::numeric_limits<float>::epsilon(),"Aspect ratio is zero or invalid");

		const float tanHalfFovy = std::tan(m_fov / 2.f);
		m_projectionMatrix = glm::mat4{0.0f};
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix[2][2] = m_camFar / (m_camFar - m_camNear);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(m_camFar * m_camNear) / (m_camFar - m_camNear);

		m_projectionMatrix[1][1] *= -1;
	}

	void Camera::setViewDirection(glm::vec3 direction) {
		VC_PROFILER_FUNCTION();
		const glm::vec3 up = {0.0f, 1.0f, 0.0f};
		const glm::vec3 w{glm::normalize(direction)};
		const glm::vec3 u{glm::normalize(glm::cross(w, up))};
		const glm::vec3 v{glm::cross(w, u)};

		m_viewMatrix = glm::mat4{1.f};
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, m_transform.position);
		m_viewMatrix[3][1] = -glm::dot(v, m_transform.position);
		m_viewMatrix[3][2] = -glm::dot(w, m_transform.position);

		recalculateTransformationMatrix();
	}

	void Camera::setViewTarget(glm::vec3 target) {
		VC_PROFILER_FUNCTION();
		setViewDirection(target - m_transform.position);
	}
}
