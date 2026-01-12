#ifndef VECTRIXWORKSPACE_TIMESTEP_H
#define VECTRIXWORKSPACE_TIMESTEP_H
namespace Vectrix {
    class DeltaTime
    {
    public:
        DeltaTime(float time = 0.0f)
            : m_Time(time)
        {
        }

        operator float() const { return m_Time; }

        [[nodiscard]] float getSeconds() const { return m_Time; }
        [[nodiscard]] float getMilliseconds() const { return m_Time * 1000.0f; }
    private:
        float m_Time;
    };
}
#endif //VECTRIXWORKSPACE_TIMESTEP_H