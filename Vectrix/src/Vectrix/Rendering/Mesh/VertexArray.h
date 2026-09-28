#ifndef VECTRIXWORKSPACE_VERTEXARRAY_H
#define VECTRIXWORKSPACE_VERTEXARRAY_H

#include "MeshHandle.h"
#include "Vectrix/Rendering/Buffer.h"

/**
 * @file VertexArray.h
 * @brief Definition of the VertexArray class
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief Where a mesh's geometry lives on the GPU
     *
     * Every mesh is uploaded once into the renderer's shared vertex/index buffers; the vertex array
     * holds the backend handle locating it there, which is what Renderer::submit draws from.
     */
    class VertexArray {
    public:
        virtual ~VertexArray() = default;

        /**
         * @brief This function create a new VertexArray instance
         * @return A shared_ptr to the newly created VertexArray
         */
        static std::shared_ptr<VertexArray> create();
    protected:
        friend class Mesh;
        virtual void setHandle(MeshHandle handle) = 0;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VERTEXARRAY_H