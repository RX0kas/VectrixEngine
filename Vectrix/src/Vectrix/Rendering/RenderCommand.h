#pragma once

#include "RendererAPI.h"
#include "Models/VertexArray.h"

/**
 * @file RenderCommand.h
 * @brief Defines the RenderCommand class for issuing rendering commands
 * @ingroup renderer
 */

namespace Vectrix
{
	/**
	 * @brief Provides static methods for issuing rendering commands
	 * @details The RenderCommand class offers a static interface for common
	 * rendering operations
	 * This class is designed to be used throughout the rendering pipeline for basic draw calls and frame management.
	 *
	 * @note All methods are static
	 */
	class RenderCommand
	{
	public:
		/**
		 * @brief Sets the clear color for the render target
		 *
		 * Defines the color used when clearing the color buffer.
		 * This color will be used for subsequent clear operations.
		 *
		 * @param color The clear color as a 4-component vector (RGBA)
		 *
		 * @note Values should be in range [0.0, 1.0]
		 */
		static void setClearColor(const glm::vec4& color)
		{
			s_RendererAPI->setClearColor(color);
		}

		/**
		 * @brief Draws geometry using the provided vertex array
		 *
		 * @param vertexArray The vertex array containing vertex data and indices
		 *
		 * @pre A frame must have been started with setupFrame()
		 * @pre The vertex array must be properly initialized
		 */
		static void drawIndexed(const VertexArray& vertexArray)
		{
			if (!frameStarted) VC_ERROR("Trying to draw but no frame has started");
			s_RendererAPI->drawIndexed(vertexArray);
		}

	private:
		static bool setupFrame() {
			if (frameStarted) VC_ERROR("Trying to start a frame while another one is being render");
			frameStarted = true;
			return s_RendererAPI->prepareFrame();
		}

		static void endFrame() {
			if (!frameStarted) VC_ERROR("Trying to end a frame but none has started");
			s_RendererAPI->endFrame();
		}
		static void sendFrame() {
			if (!frameStarted) VC_ERROR("Trying to send a frame but none has started");
			s_RendererAPI->sendFrame();
			frameStarted = false;
		}
		friend class Application;
		static bool frameStarted;
		friend class Renderer;

		static RendererAPI* s_RendererAPI;
	};
}

