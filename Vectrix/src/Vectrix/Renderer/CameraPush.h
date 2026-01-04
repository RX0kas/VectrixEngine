#ifndef VECTRIXWORKSPACE_CAMERAUBO_H
#define VECTRIXWORKSPACE_CAMERAUBO_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "GraphicAPI/Vulkan/rendering/Device.h"

namespace Vectrix {

    struct CameraPushData {
        alignas(16) glm::mat4 transform;
    };

    class CameraPush {
    public:
        CameraPush();
        static VkPushConstantRange createPushConstantRange();
        static void sendPush(VkCommandBuffer cmd,VkPipelineLayout layout);
    private:
        Device& p_device;
    };
}

#endif //VECTRIXWORKSPACE_CAMERAUBO_H