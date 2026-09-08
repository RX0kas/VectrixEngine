#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "glm/gtc/quaternion.hpp"


/**
 * @file EditorCamera.h
 * @brief Definition of the EditorCamera class
 * @ingroup rendering
 */

namespace Vectrix {
	/**
	 * @brief The camera the editor looks through, independent of the scene
	 *
	 * Unlike Camera it is not tied to an entity, which is what lets the editor move
	 * around without touching the scene being edited. Its position, scale and rotation
	 * are public, call recalculateMatrices after changing them.
	 * @see Camera
	 * @ingroup rendering
	 */
	class EditorCamera {
	public:
		/**
		 * @brief Build the camera with its projection settings
		 * @param fov The vertical field of view, in degrees
		 * @param camNear How close something can get before it stops being drawn
		 * @param camFar How far something can get before it stops being drawn
		 */
		EditorCamera(float fov = 50.0f,float camNear = 0.1f,float camFar = 1000.0f);

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
		 * @brief This function set a custom aspect ratio to the camera
		 * @param aspect The new custom aspect ratio
		 */
		void setCustomAspect(float aspect) {
			m_customAspect = aspect;
		}

		/**
		 * @brief This function return the aspect ratio of the camera
		 */
		[[nodiscard]] float getAspect() const;

		/**
		 * @brief This function recalculate all the matrices
		 */
		void recalculateMatrices();


		/**
		 * @brief Sets the rotation of the camera in degrees
		 * @param newRotation Euler angles in degrees
		 * @see getRotationDeg
		 */
		void setRotationDeg(glm::vec3 newRotation) {
			m_rotation = glm::quat(glm::radians(newRotation));
		}


		/**
		 * @brief Returns the rotation of the model in degrees
		 **/
		[[nodiscard]] glm::vec3 getRotationDeg() const {
			return glm::degrees(glm::eulerAngles(m_rotation));
		}

		/// Where the camera sits in the world
		glm::vec3 m_position{ 0};

		/// The scale of the camera, which is left at one in normal use
		glm::vec3 m_scale{ 1.f, 1.f, 1.f };

		/// Which way the camera looks, as a quaternion
		glm::quat m_rotation = glm::identity<glm::quat>();
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

		float m_customAspect = -1;

	};

}
