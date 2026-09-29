#ifndef VECTRIXWORKSPACE_MATH_H
#define VECTRIXWORKSPACE_MATH_H

/**
 * @file Math.h
 * @brief How GLM is configured for the whole engine
 * @ingroup utils
 *
 * Include this rather than GLM directly, so every translation unit sees the same configuration.
 */

#ifndef M_PI
/**
 * @brief Pi, defined here only when the standard library did not already
 * @ingroup utils
 */
#define M_PI 3.14159265358979323846
#endif

/// @cond INTERNAL
// How GLM is configured for the whole engine: angles in radians and a 0 to 1 depth
// range, which is what Vulkan expects.
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
/// @endcond
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Vectrix {
	/**
	 * @brief Copy a vector into a vector type of another library
	 *
	 * Mostly used to hand a GLM vector to ImGui and the other way round.
	 * @tparam T The vector type to produce
	 * @tparam F The vector type to read from
	 * @tparam N How many components to copy
	 * @param vec The vector to copy
	 * @return The same components, in the wanted type
	 * @ingroup utils
	 */
	template<typename T,typename F, int N>
	T changeVecType(F vec) {
		T nVec;
		for (int i = 0; i < N; ++i) {
			nVec[i] = vec[i];
		}
		return nVec;
	}
}

#endif //VECTRIXWORKSPACE_MATH_H
