#include "MeshRendererComponent.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    MeshRendererComponent::MeshRendererComponent(std::shared_ptr<Entity> entity,const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Shader> &shader, const std::shared_ptr<Texture>& texture) {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(shader,"The shader is a nullptr");
        VC_CORE_ASSERT(texture,"The texture is a nullptr");
        VC_CORE_ASSERT(mesh,"The mesh is a nulltr");
        this->shader = shader;
        this->texture = texture;
        this->mesh = mesh;
    }

    MeshRendererComponent::MeshRendererComponent(std::shared_ptr<Entity> entity) : enable(false) {}
} // Vectrix