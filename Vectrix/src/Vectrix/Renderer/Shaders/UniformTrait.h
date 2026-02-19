#ifndef VECTRIXWORKSPACE_UNIFORMTRAIT_H
#define VECTRIXWORKSPACE_UNIFORMTRAIT_H
#include "ShaderUniform.h"
#include "glm/glm.hpp"


namespace Vectrix {
    template<typename T>
    struct UniformTraits;

    #define CREATE_UNIFORM_TRAITS(Ctype,SUType)template<> struct UniformTraits<Ctype> {static constexpr ShaderUniformType type = ShaderUniformType::SUType;static constexpr bool valid=true;};

    CREATE_UNIFORM_TRAITS(bool,Bool)
    CREATE_UNIFORM_TRAITS(float,Float)

    CREATE_UNIFORM_TRAITS(int,Int)
    CREATE_UNIFORM_TRAITS(unsigned int,Uint)
    CREATE_UNIFORM_TRAITS(glm::vec2,Vec2)
    CREATE_UNIFORM_TRAITS(glm::vec3,Vec3)
    CREATE_UNIFORM_TRAITS(glm::vec4,Vec4)
    CREATE_UNIFORM_TRAITS(glm::mat4,Mat4)
}

#endif //VECTRIXWORKSPACE_UNIFORMTRAIT_H