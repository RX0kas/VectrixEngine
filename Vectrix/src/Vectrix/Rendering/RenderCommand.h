#pragma once

#include "RendererAPI.h"
#include "Mesh/VertexArray.h"
#include "Vectrix/Debug/Profiler.h"

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
			VC_PROFILER_FUNCTION();
			s_rendererAPI->setClearColor(color);
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
			VC_PROFILER_FUNCTION();
			if (!s_frameStarted) VC_ERROR("Trying to draw but no frame has started");
			s_rendererAPI->drawIndexed(vertexArray);
		}

	private:
		static bool canRender() {
			VC_PROFILER_FUNCTION();
			s_frameStarted = true;
			return s_rendererAPI->canRender();
		}

		static void beginFrame() {
			s_rendererAPI->beginFrame();
		}

		static void endFrame() {
			VC_PROFILER_FUNCTION();
			if (!s_frameStarted) VC_ERROR("Trying to end a frame but none has started");
			s_rendererAPI->endFrame();
		}
		static void sendFrame() {
			VC_PROFILER_FUNCTION();
			if (!s_frameStarted) VC_ERROR("Trying to send a frame but none has started");
			s_rendererAPI->sendFrame();
			s_frameStarted = false;
		}
		friend class Application;
		friend class Renderer;

		static bool s_frameStarted;
		static RendererAPI* s_rendererAPI;
	};
}

