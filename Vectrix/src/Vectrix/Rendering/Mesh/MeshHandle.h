#ifndef VECTRIXWORKSPACE_MESHHANDLE_H
#define VECTRIXWORKSPACE_MESHHANDLE_H
#include <cstdint>

namespace Vectrix {
    /// @cond INTERNAL
    /**
     * @brief Used for batch rendering
     **/
    struct MeshHandle {
        uint32_t firstVertex = 0;
        uint32_t firstIndex = 0;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
    };
    /// @endcond
}
#endif //VECTRIXWORKSPACE_MESHHANDLE_H