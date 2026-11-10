#ifndef VECTRIXWORKSPACE_FIXEDTIMESTEP_H
#define VECTRIXWORKSPACE_FIXEDTIMESTEP_H

#include <cmath>
#include <cstdint>

/**
 * @file FixedTimestep.h
 * @brief Definition of FixedTimestep, which turns frame times into a number of fixed steps
 * @ingroup core
 */

namespace Vectrix {
	/**
	 * @brief Counts how many fixed steps the elapsed time adds up to (an accumulator)
	 *
	 * What the Application runs Layer::OnFixedUpdate with: each frame, advance gives how many steps to run, and
	 * the time left over is kept for the next frame, so the steps keep pace with real time whatever the frame
	 * rate. Past maxSteps in a frame, the rest is dropped (the simulation slows down instead of spending ever
	 * more time catching up).
	 * @ingroup core
	 */
	class FixedTimestep {
	public:
		/**
		 * @param step The length of a step in seconds, above 0
		 * @param maxSteps The most steps a frame may run, at least 1
		 */
		explicit FixedTimestep(double step = 1.0 / 60.0, uint32_t maxSteps = 8) { configure(step, maxSteps); }

		/// @brief Change the step and the limit; values out of range keep the previous ones
		void configure(double step, uint32_t maxSteps) {
			if (step > 0.0)
				m_step = step;
			if (maxSteps > 0)
				m_maxSteps = maxSteps;
		}

		/**
		 * @brief Add a frame's time
		 * @param elapsed The frame's time in seconds (negative is taken as 0)
		 * @return How many steps to run now
		 */
		uint32_t advance(double elapsed) {
			if (elapsed > 0.0)
				m_accumulator += elapsed;
			uint32_t steps = 0;
			while (m_accumulator >= m_step) {
				if (steps == m_maxSteps) {
					m_accumulator = std::fmod(m_accumulator, m_step);
					break;
				}
				m_accumulator -= m_step;
				++steps;
			}
			return steps;
		}

		/// @brief How far the time is between the last step and the next, from 0 to 1
		[[nodiscard]] double alpha() const { return m_accumulator / m_step; }
		/// @brief The length of a step, in seconds
		[[nodiscard]] double step() const { return m_step; }
		/// @brief The most steps a frame runs
		[[nodiscard]] uint32_t maxSteps() const { return m_maxSteps; }
		/// @brief Forget the time left over
		void reset() { m_accumulator = 0.0; }
	private:
		double m_step = 1.0 / 60.0;
		uint32_t m_maxSteps = 8;
		double m_accumulator = 0.0;
	};
}

#endif //VECTRIXWORKSPACE_FIXEDTIMESTEP_H
