#ifndef VECTRIXWORKSPACE_MATH_H
#define VECTRIXWORKSPACE_MATH_H

#define VC_2PI 6.28318530717948316 // 2*PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

template<typename T>
constexpr T rad(T deg) {
    return deg*0.01745329252; // PI/180
}
template<typename T>
constexpr T deg(T rad) {
    return rad*57.2957795131; // 180/PI
}
#endif //VECTRIXWORKSPACE_MATH_H