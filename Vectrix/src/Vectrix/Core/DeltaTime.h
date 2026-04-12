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
        DeltaTime(float time = 0.0f) : m_Time(time) {}

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