#include "vcpch.h"

#include "Camera.h"

#include <cmath>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Framebuffer.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
	std::shared_ptr<Entity> Camera::s_currentCamera = nullptr;

	Camera::Camera(std::shared_ptr<Entity> entity, float fov,float camNear,float camFar) : m_viewMatrix(1.0f),m_fov(glm::radians(fov)),m_camFar(camFar),m_camNear(camNear) {
		m_entity = entity;
		recalculateMatrices();
		setAsCurrent();
	}

	TransformComponent& Camera::transform() const {
		return m_entity->getComponent<TransformComponent>();
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

	void Camera::recalculateViewMatrix() {
		m_viewMatrix = glm::inverse(transform().modelMatrix());
	}


	void Camera::recalculateTransformationMatrix() {
		m_transformationMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void Camera::recalculateProjectionMatrix() {
		float aspect = getAspect();

		VC_CORE_ASSERT(aspect > std::numeric_limits<float>::epsilon(),"Aspect ratio is invalid");

		m_projectionMatrix = glm::perspectiveRH_ZO(m_fov,aspect,m_camNear,m_camFar);

		m_projectionMatrix[1][1] *= -1.0f;
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
		const glm::vec3 position = transform().position;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);

		recalculateTransformationMatrix();
	}

	void Camera::setViewTarget(glm::vec3 target) {
		VC_PROFILER_FUNCTION();
		setViewDirection(target - transform().position);
	}

	std::shared_ptr<Entity> Camera::getCurrentCamera() { return s_currentCamera; }

	void Camera::setAsCurrent() { s_currentCamera = m_entity; }

	[[nodiscard]] bool Camera::isCurrent() const { return m_entity==s_currentCamera; }
}
