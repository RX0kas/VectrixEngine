#pragma once

#include "Vectrix/Rendering/RendererAPI.h"

#include "Camera/PerspectiveCamera.h"
#include "Shaders/Shader.h"
#include "Mesh/VertexArray.h"
#include "Vectrix/Scene/Components/TransformComponent.h"

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
		struct SceneData {
			/**
			 * @brief The scene camera
			 */
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
		 * @param modelMatrix The object model matrix
		 * @param textureIndex The index of the texture inside the shader
		 */
		static void submit(Shader& shader,const std::shared_ptr<VertexArray>& vertexArray, glm::mat4 modelMatrix,uint32_t textureIndex = 0);

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
		static std::unique_ptr<SceneData> m_SceneData;
	};
}
