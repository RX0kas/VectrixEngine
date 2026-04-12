#pragma once

#include "glm/glm.hpp"

/**
 * @file Vertex.h
 * @brief Definition of the vertex struct
 * @ingroup mesh
 */

namespace Vectrix {
	/**
	 * @brief Describe how a vertex is stored
	 */
	struct Vertex {
		/**
		 * @brief This is the model position
		 */
		glm::vec3 position;
		/**
		 * @brief This is the normal vector
		 */
		glm::vec3 normal;
		/**
		 * @brief This is the texture coordinates
		 */
		glm::vec2 uv;

		/**
		 * @brief Verify if two Vertex are the same
		 * @param other The other vertex
		 */
		bool operator==(const Vertex& other) const {
			return position == other.position && normal == other.normal && uv == other.uv;
		}
	};
} // namespace Vectrix