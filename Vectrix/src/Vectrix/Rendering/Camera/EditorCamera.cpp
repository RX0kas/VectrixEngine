#include "vcpch.h"

#include "EditorCamera.h"

#include <cmath>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Framebuffer.h"

namespace Vectrix {

	EditorCamera::EditorCamera(float fov,float camNear,float camFar) : m_viewMatrix(1.0f),m_fov(glm::radians(fov)),m_camFar(camFar),m_camNear(camNear) {
		recalculateMatrices();
	}

	float EditorCamera::getAspect() const {
		return m_customAspect!=-1 ? m_customAspect : Application::instance().window().getAspect();
	}

	void EditorCamera::recalculateMatrices() {
		VC_PROFILER_FUNCTION();
		recalculateProjectionMatrix();
		recalculateViewMatrix();
		recalculateTransformationMatrix();
	}

	void EditorCamera::recalculateViewMatrix() {
		VC_PROFILER_FUNCTION();
		m_viewMatrix = glm::inverse(TransformComponent::modelMatrix(m_position,m_scale,m_rotation));
	}


	void EditorCamera::recalculateTransformationMatrix() {
		m_transformationMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void EditorCamera::recalculateProjectionMatrix() {
		VC_PROFILER_FUNCTION();
		float aspect = getAspect();
		VC_CORE_ASSERT(aspect > std::numeric_limits<float>::epsilon(),"Aspect ratio is invalid");

		m_projectionMatrix = glm::perspectiveRH_ZO(m_fov,aspect,m_camNear,m_camFar);

		m_projectionMatrix[1][1] *= -1.0f;
	}

	void EditorCamera::setViewDirection(glm::vec3 direction) {
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
		m_viewMatrix[3][0] = -glm::dot(u, m_position);
		m_viewMatrix[3][1] = -glm::dot(v, m_position);
		m_viewMatrix[3][2] = -glm::dot(w, m_position);

		recalculateTransformationMatrix();
	}

	void EditorCamera::setViewTarget(glm::vec3 target) {
		VC_PROFILER_FUNCTION();
		setViewDirection(target - m_position);
	}
}
