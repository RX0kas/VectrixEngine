#ifndef VECTRIXWORKSPACE_CAMERAUBO_H
#define VECTRIXWORKSPACE_CAMERAUBO_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "GraphicAPI/Vulkan/rendering/Device.h"
#include "Vectrix/Renderer/Models/Transform.h"

namespace Vectrix {
    // Max size 128 bytes
    struct CameraPushData {
        alignas(16) glm::mat4 transform;
        alignas(16) glm::mat4 modelMat;
    };

    class CameraPush {
    public:
        CameraPush();
        static VkPushConstantRange createPushConstantRange();
        static void sendPush(VkCommandBuffer cmd,VkPipelineLayout layout,Transform transform);
    private:
        Device& p_device;
    };
}

#endif //VECTRIXWORKSPACE_CAMERAUBO_H