#pragma once

#include <glm/glm.hpp>


namespace Vectrix {
	class PerspectiveCamera
	{
	public:
		PerspectiveCamera();

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
	private:
		void recalculateViewMatrix();
		void recalculateProjectionMatrix();
		void recalculateTransformationMatrix();
	private:
		friend class Renderer;
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_transformationMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f };
	};

}
