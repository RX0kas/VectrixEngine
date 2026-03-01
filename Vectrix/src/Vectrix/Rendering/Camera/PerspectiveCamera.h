#pragma once

#include <glm/glm.hpp>

namespace Vectrix {
	class PerspectiveCamera
	{
	public:
		PerspectiveCamera(float fov = 50.0f,float camNear = 0.1f,float camFar = 10.0f);

		[[nodiscard]] const glm::vec3& getPosition() const { return m_Position; }
		void setPosition(const glm::vec3& position) { m_Position = position; recalculateMatrices(); }

		[[nodiscard]] glm::vec3 getRotation() const { return m_Rotation; }
		void setRotation(glm::vec3 rotation) { m_Rotation = rotation; recalculateMatrices(); }

		[[nodiscard]] const glm::mat4& getProjectionMatrix() const { return m_ProjectionMatrix; }
		[[nodiscard]] const glm::mat4& getViewMatrix() const { return m_ViewMatrix; }
		[[nodiscard]] const glm::mat4& getTransformationMatrix() const { return m_transformationMatrix; }
		void setViewDirection(glm::vec3 position, glm::vec3 direction);
		void setViewTarget(glm::vec3 position, glm::vec3 target);
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
		void recalculateMatrices();

		[[nodiscard]] float getFOV() const {
			return m_fov;
		}

		void setFOV(float fov) {
			this->m_fov = fov;
		}

		[[nodiscard]] float getCamNear() const {
			return m_camNear;
		}

		void setCamNear(float camNear) {
			this->m_camNear = camNear;
		}

		[[nodiscard]] float getCamFar() const {
			return m_camFar;
		}

		void setCamFar(float camFar) {
			this->m_camFar = camFar;
		}

	private:
		void recalculateViewMatrix();
		void recalculateProjectionMatrix();
		void recalculateTransformationMatrix();
	private:
		friend class Renderer;

		float m_fov,m_camNear,m_camFar;

		glm::mat4 m_ProjectionMatrix{};
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_transformationMatrix{};

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f };
	};

}
