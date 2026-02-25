#pragma once

#include "Vectrix/Rendering/RendererAPI.h"

#include "Camera/PerspectiveCamera.h"
#include "Models/Model.h"
#include "Shaders/Shader.h"
#include "Models/VertexArray.h"

/**
 * @file Renderer.h
 * @brief Main renderer class definition
 * @ingroup renderer
 */

namespace Vectrix {
	/**
	 * @class Renderer
	 * @brief Central rendering manager for 3D scenes
	 *
	 * Provides static methods for scene-based rendering. All rendering
	 * must occur between BeginScene() and EndScene() calls.
	 */
	class Renderer
	{
	public:
		/**
		 * @struct SceneData
		 * @brief Per-scene rendering data
		 */
		struct SceneData
		{
			PerspectiveCamera* camera;
		};
		/**
		 * @brief Start rendering a scene with the given camera
		 * @param camera Camera to use for view/projection matrices
		 */
		static void beginScene(PerspectiveCamera& camera);
		/**
		 * @brief End the current rendering scene
		 */
		static void endScene();

		/**
		 * @brief Submit geometry with transform
		 * @param shader Shader to use
		 * @param vertexArray Geometry to render
		 * @param transform Transform to apply (position, scale, rotation)
		 * @deprecated Will be removed when the material system will be created, please use @ref Model and @ref Vectrix::Renderer::submit(Shader& shader, Model& model) instead
		 */
		static void submit(Shader& shader, const VertexArray& vertexArray,const Transform& transform=Transform{glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)});

		/**
		 * @brief Submit a complete model
		 * @param shader Shader to use
		 * @param model Model to render
		 */
		static void submit(Shader& shader, Model& model);

		/**
		 * @brief Get current graphics API
		 * @return The active rendering API
		 */
		static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }
		/**
		 * @brief Get current scene data
		 * @return Reference to active scene data
		 */
		static SceneData& getSceneData() {return *m_SceneData;}
	private:
		static Own<SceneData> m_SceneData;
	};

}
