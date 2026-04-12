#pragma once

#include <glm/glm.hpp>

/**
 * @file PerspectiveCamera.h
 * @brief Definition of the camera class
 * @ingroup rendering
 */

namespace Vectrix {
	/**
	 * @brief This class is the camera class
	 */
	class PerspectiveCamera	{
	public:
		/**
		 * @brief This creates a new camera with a perspective projection
		 * @param fov The camera FOV
		 * @param camNear The camera near
		 * @param camFar The camera far
		 */
		PerspectiveCamera(float fov = 50.0f,float camNear = 0.1f,float camFar = 10.0f);

		/**
		 * @brief This function return the current world position of the camera
		 */
		[[nodiscard]] const glm::vec3& getPosition() const { return m_position; }

		/**
		 * @brief This function sets the new world position of the camera
		 * @param position The new position
		 */
		void setPosition(const glm::vec3& position) { m_position = position; recalculateMatrices(); }

		/**
		 * @brief This function return the current rotation of the camera
		 */
		[[nodiscard]] glm::vec3 getRotation() const { return m_rotation; }
		/**
		 * @brief This function sets the new rotation of the camera
		 * @param rotation The new rotation (Radians)
		 */
		void setRotation(glm::vec3 rotation) { m_rotation = rotation; recalculateMatrices(); }

		/**
		 * @brief This function return the projection matrix
		 */
		[[nodiscard]] const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
		/**
		 * @brief This function return the view matrix
		 */
		[[nodiscard]] const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
		/**
		 * @brief This function return the projection matrix * the view matrix
		 */
		[[nodiscard]] const glm::mat4& getTransformationMatrix() const { return m_transformationMatrix; }

		/**
		 * @brief This function change the direction of the camera to look at something from the direction of the object
		 * @param direction The new direction
		 */
		void setViewDirection(glm::vec3 direction);

		/**
		 * @brief This function change the direction of the camera to look at something from the position of the object
		 * @param target The target
		 */
		void setViewTarget(glm::vec3 target);

		/**
		 * @brief This function return the camera FOV
		 */
		[[nodiscard]] float getFOV() const {
			return m_fov;
		}

		/**
		 * @brief This function change the camera FOV
		 * @param fov The new FOV
		 */
		void setFOV(float fov) {
			this->m_fov = fov;
			recalculateMatrices();
		}

		/**
		 * @brief This function return the camera near
		 */
		[[nodiscard]] float getCamNear() const {
			return m_camNear;
		}

		/**
		 * @brief This function change the camera near
		 * @param camNear The new cameraFar
		 */
		void setCamNear(float camNear) {
			this->m_camNear = camNear;
			recalculateMatrices();
		}

		/**
		 * @brief This function return the camera Far
		 */
		[[nodiscard]] float getCamFar() const {
			return m_camFar;
		}

		/**
		 * @brief This function change the camera far
		 * @param camFar The new cameraFar
		 */
		void setCamFar(float camFar) {
			this->m_camFar = camFar;
			recalculateMatrices();
		}

		/**
		 * @brief This function recalculate all the matrices
		 */
		void recalculateMatrices();
	private:
		void recalculateViewMatrix();
		void recalculateProjectionMatrix();
		void recalculateTransformationMatrix();
	private:
		friend class Renderer;

		float m_fov,m_camNear,m_camFar;

		glm::mat4 m_projectionMatrix{};
		glm::mat4 m_viewMatrix;
		glm::mat4 m_transformationMatrix{};

		glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_rotation = { 0.0f, 0.0f, 0.0f };
	};

}
