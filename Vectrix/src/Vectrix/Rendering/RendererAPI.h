#pragma once

#include <glm/glm.hpp>
#include "Mesh/VertexArray.h"

/**
 * @file RendererAPI.h
 * @brief Internal renderer class
 * @ingroup renderer
 */

namespace Vectrix {
	/**
	 * @brief This class is a virtual class that is called by RenderCommand
	 */
	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		/**
		 * @brief The different graphics API
		 */
		enum class API
		{
			None = 0, Vulkan = 1
		};
		/// @cond INTERNAL
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual bool canRender() = 0;
		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void sendFrame() = 0;

		virtual void drawIndexed(const VertexArray& vertexArray) = 0;
		/// @endcond

		/**
		 * @brief This function return the current graphics API used
		 */
		static API getAPI() { return s_API; }
	private:
		static API s_API;
	};

}
