#ifndef VECTRIXWORKSPACE_MESHHANDLE_H
#define VECTRIXWORKSPACE_MESHHANDLE_H
#include <cstdint>

namespace Vectrix {
    /// @cond INTERNAL
    /**
     * @brief Used for batch rendering
     **/
    struct MeshHandle {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t vertexOffset;
    };
    /// @endcond
}
#endif //VECTRIXWORKSPACE_MESHHANDLE_H