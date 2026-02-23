#pragma once

#include "RendererAPI.h"
#include "Models/VertexArray.h"

/**
 * @file RenderCommand.h
 * @brief Definition of the RenderCommand class
 * @ingroup rendering
 */

namespace Vectrix
{
	class RenderCommand
	{
	public:
		static void setClearColor(const glm::vec4& color)
		{
			s_RendererAPI->setClearColor(color);
		}

		static void drawIndexed(const VertexArray& vertexArray)
		{
			if (!frameStarted) VC_ERROR("Trying to draw but no frame has started");
			s_RendererAPI->drawIndexed(vertexArray);
		}
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
	private:
		static bool frameStarted;
		friend class Renderer;

		static RendererAPI* s_RendererAPI;
	};
}

