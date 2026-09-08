#ifndef VECTRIXWORKSPACE_TIMESTEP_H
#define VECTRIXWORKSPACE_TIMESTEP_H

/**
 * @file DeltaTime.h
 * @brief Definition of the DeltaTime class
 * @ingroup core
 */


namespace Vectrix {
    /**
     * @brief This class represent how many times have past since the last frame
     */
    class DeltaTime {
    public:
        /**
         * @brief Build a delta time from a duration in seconds
         * @param time The elapsed time in seconds
         */
        DeltaTime(float time = 0.0f) : m_Time(time) {}

        /**
         * @brief Convert to a float, so it can be used directly in a computation
         * @return The elapsed time in seconds
         * @see getSeconds
         */

        operator float() const { return m_Time; }

        /**
         * @brief This function return how much time as past in seconds
         */
        [[nodiscard]] float getSeconds() const { return m_Time; }
        /**
         * @brief This function return how much time as past in milliseconds
         */
        [[nodiscard]] float getMilliseconds() const { return m_Time * 1000.0f; }
    private:
        float m_Time;
    };
}
#endif //VECTRIXWORKSPACE_TIMESTEP_H