#ifndef VECTRIXWORKSPACE_FRAMEBUFFER_H
#define VECTRIXWORKSPACE_FRAMEBUFFER_H
#include <memory>
#include <cstdint>

#include "imgui.h"
#include "glm/vec2.hpp"

namespace Vectrix {
    /**
     * @brief This struct represent the information about a framebuffer
     */
    struct FramebufferSpecification {
        /**
         * @brief The width of the framebuffer
         */
        std::uint32_t width = -1;
        /**
         * @brief The height of the framebuffer
         */
        std::uint32_t height = -1;
    };

    /**
     * @brief This class can be used to draw on an image
     */
    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        /**
         * @brief This function tell the renderer to draw inside this framebuffer
         * @pre Must not be already bound
         */
        virtual void bind() = 0;

        /**
         * @brief This function tell the renderer to stop drawing inside this framebuffer
         * @pre Must be bound
         * @warning This function must be called before you try
         */
        virtual void unbind() = 0;

        /**
         * @brief This function return the ImGui textureID of the Framebuffer
         */
        [[nodiscard]] virtual ImTextureID getTextureID() const = 0;

        /**
         * @brief This function return if the framebuffer is currently bound
         */
        [[nodiscard]] virtual bool isBound() const = 0;

        /**
         * @brief This function return the specification of the Framebuffer
         */
        [[nodiscard]] virtual const FramebufferSpecification& getSpecification() const = 0;


        /**
         * @brief This function change the size of a framebuffer
         * @param size The new size
         */
        virtual void resize(glm::vec2 size) = 0;

        /**
         * @brief This function can be used to create a framebuffer
         * @param spec The specification of the framebuffer
         */
        static std::shared_ptr<Framebuffer> create(const FramebufferSpecification& spec);

        /**
         * @brief This function return the current framebuffer
         * @warning This function return nullptr if no framebuffer is currently bound
         */
        [[nodiscard]] static Framebuffer* getCurrentFramebuffer() { return s_currentFramebuffer; }

        /**
         * @brief This function return true if a framebuffer is currently bound
         */
        [[nodiscard]] static bool isAFramebufferActive() { return s_currentFramebuffer!=nullptr; }

        [[nodiscard]] virtual float getAspectRatio() const = 0;
    protected:
        static Framebuffer* s_currentFramebuffer;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_FRAMEBUFFER_H