#include "CameraPush.h"

#include "Vectrix/Renderer/Renderer.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"


namespace Vectrix {
    CameraPush::CameraPush() : p_device(VulkanContext::instance().getDevice()) {
        VC_ASSERT(sizeof(CameraPushData)<=128,"The size of the push constant must be 128 bytes maximum");
    }

    VkPushConstantRange CameraPush::createPushConstantRange() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(CameraPushData);
        return pushConstantRange;
    }

    void CameraPush::sendPush(VkCommandBuffer cmd,VkPipelineLayout layout,Transform transform) {
        CameraPushData push{};
        push.transform  = Renderer::getSceneData().camera->getTransformationMatrix();
        push.modelMat = transform.mat4();
        vkCmdPushConstants(
            cmd,
            layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(CameraPushData),
            &push);
    }
}
