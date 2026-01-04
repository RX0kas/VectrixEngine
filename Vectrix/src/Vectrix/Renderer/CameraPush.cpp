#include "CameraPush.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"


namespace Vectrix {
    CameraPush::CameraPush() : p_device(VulkanContext::instance().getDevice()) {}

    VkPushConstantRange CameraPush::createPushConstantRange() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(CameraPushData);
        return pushConstantRange;
    }

    void CameraPush::sendPush(VkCommandBuffer cmd,VkPipelineLayout layout) {
        CameraPushData push{};
        push.transform  = Application::instance().getCamera().getTransformationMatrix();

        vkCmdPushConstants(
            cmd,
            layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(CameraPushData),
            &push);
    }
}
