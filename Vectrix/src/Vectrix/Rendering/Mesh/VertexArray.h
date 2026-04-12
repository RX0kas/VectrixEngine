#ifndef VECTRIXWORKSPACE_VERTEXARRAY_H
#define VECTRIXWORKSPACE_VERTEXARRAY_H

#include "Vectrix/Rendering/Buffer.h"

/**
 * @file VertexArray.h
 * @brief Definition of the VertexArray class
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief The combination of an IndexBuffer (optional) and a VertexBuffer
     */
    class VertexArray {
    public:
        virtual ~VertexArray() = default;

        /**
         * @brief This function permit to use the vertex array
         */
        virtual void bind() const = 0;

        /**
         * @brief This function add a VertexBuffer to the VertexArray
         * @param vertexBuffer A shared_ptr to the VertexBuffer class you wanna add
         */
        virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
        /**
         * @brief This function add a IndexBuffer to the VertexArray
         * @param indexBuffer A shared_ptr to the IndexBuffer class you wanna add
         */
        virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

        /**
         * @brief This function return all the VertexBuffer that this VertexArray has
         * @return All the VertexBuffer that this VertexArray has
         */
        [[nodiscard]] virtual std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers() const = 0;
        /**
         * @brief This function return the IndexBuffer that this VertexArray has
         * @return The IndexBuffer that this VertexArray has
         */
        [[nodiscard]] virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const = 0;

        /**
         * @brief This function create a new VertexArray instance
         * @return A shared_ptr to the newly created VertexArray
         */
        static std::shared_ptr<VertexArray> create();
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VERTEXARRAY_H