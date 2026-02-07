#pragma once

#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "Vectrix/Renderer/Models/Vertex.h"

#include <vector>

#include "Vectrix/Renderer/Buffer.h"

namespace Vectrix {

    struct PipelineConfigInfo {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo() = default;

        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
		BufferLayout layout;
    };

    class Pipeline {
    public:
        Pipeline(Device& device,const std::vector<uint32_t>& vertCode,const std::vector<uint32_t>& fragCode,const PipelineConfigInfo& configInfo);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

        void bind(VkCommandBuffer commandBuffer) const;

        [[nodiscard]] VkPipeline getPipeline() const { return m_graphicsPipeline; }
        static void enableAlphaBlending(PipelineConfigInfo& configInfo);

    private:
        void createGraphicsPipeline(const std::vector<uint32_t>& vertCode,const std::vector<uint32_t>& fragCode,const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<uint32_t>& code,VkShaderModule* shaderModule);

        Device& m_device;
        VkPipeline m_graphicsPipeline;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragShaderModule;
    };
}
