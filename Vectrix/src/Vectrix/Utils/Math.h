#ifndef VECTRIXWORKSPACE_MATH_H
#define VECTRIXWORKSPACE_MATH_H

/**
 * @file Math.h
 * @brief The small maths helpers the engine uses, on top of GLM
 * @ingroup utils
 */

/**
 * @brief Two times pi, a full turn in radians
 * @ingroup utils
 */
#define VC_2PI 6.28318530717948316 // 2*PI
#include "imgui.h"
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
	 * @brief Convert an angle from degrees to radians
	 * @tparam T The type of the angle
	 * @param deg The angle in degrees
	 * @return The same angle in radians
	 * @see deg
	 * @ingroup utils
	 */
	template<typename T>
	constexpr T rad(T deg) {
		return deg*0.01745329252; // PI/180
	}
	/**
	 * @brief Convert an angle from radians to degrees
	 * @tparam T The type of the angle
	 * @param rad The angle in radians
	 * @return The same angle in degrees
	 * @see rad
	 * @ingroup utils
	 */
	template<typename T>
	constexpr T deg(T rad) {
		return rad*57.2957795131; // 180/PI
	}


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

	/**
	 * @brief Split a model matrix back into a position, a rotation and a scale
	 *
	 * It is what lets a gizmo hand back a matrix that the editor can write into a
	 * TransformComponent. Shear and perspective are dropped.
	 * @param transform The matrix to take apart
	 * @param translation Receives the position
	 * @param rotation Receives the rotation, as Euler angles in radians
	 * @param scale Receives the scale on each axis
	 * @return true when the matrix could be taken apart
	 * @note Adapted from `glm::decompose`
	 * @ingroup utils
	 */
	inline bool decomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale) {
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = static_cast<float>(atan2(Row[1][2], Row[2][2]));
			rotation.z = static_cast<float>(atan2(Row[0][1], Row[0][0]));
		}
		else {
			rotation.x = static_cast<float>(atan2(-Row[2][0], Row[1][1]));
			rotation.z = 0;
		}

		return true;
	}
}
#endif //VECTRIXWORKSPACE_MATH_H