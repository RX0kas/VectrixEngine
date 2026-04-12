#ifndef VECTRIXWORKSPACE_MODEL_H
#define VECTRIXWORKSPACE_MODEL_H
#include "Transform.h"
#include "VertexArray.h"
#include "Vectrix/Utils/Math.h"

/**
* @file Model.h
* @brief Definition of the Model class and internal struct
* @ingroup mesh
*/

namespace Vectrix {
    /// @cond INTERNAL
    /**
     * @brief Used for batch rendering
     **/
    struct MeshHandle {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t  vertexOffset;
    };
    /// @endcond

    /**
     * @brief Represents a 3D model with its geometry, layout, and transform
     *
     * @note Models cannot be constructed directly, use MeshManager::createModel or MeshManager::loadModel to create them
     */
    class Model {
    public:
        /**
         * @brief Returns the vertex array containing the GPU buffers of this model
         **/
        [[nodiscard]] std::shared_ptr<VertexArray> getVertexArray() const { return m_vertexArray; }

        /**
         * @brief Returns the buffer layout describing the vertex attributes
         **/
        [[nodiscard]] BufferLayout getLayout() const { return m_layout; }

        /**
         * @brief Sets the world position of the model
         * @param position Position in world space
         **/
        void setPosition(glm::vec3 position) { m_transform.translation=position; }
        /**
         * @brief Sets the rotation of the model in radians
         * @param rotation Euler angles in radians
         */
        void setRotation(glm::vec3 rotation) { m_transform.rotation=rotation; }

        /**
         * @brief Sets the rotation of the model in degrees
         * @param rotation Euler angles in degrees
         */
        void setRotationDeg(const glm::vec3 rotation) {
            m_transform.rotation.x=rad(rotation.x);
            m_transform.rotation.y=rad(rotation.y);
            m_transform.rotation.z=rad(rotation.z);
        }
        /**
         * @brief Sets the scale of the model
         * @param scale Scale factors on each axis
         **/
        void setScale(const glm::vec3 scale) { m_transform.scale=scale; }

        /** @brief Returns the world position of the model */
        [[nodiscard]] glm::vec3 getPosition() const { return m_transform.translation; }

        /**
         * @brief Returns the rotation of the model in radians
         **/
        [[nodiscard]] glm::vec3 getRotation() const { return m_transform.rotation; }

        /**
         * @brief Returns the rotation of the model in degrees
         **/
        [[nodiscard]] glm::vec3 getRotationDeg() const {
            return {deg(m_transform.rotation.x),deg(m_transform.rotation.y),deg(m_transform.rotation.z)};
        }

        /**
         * @brief Returns the scale of the model
         */
        [[nodiscard]] glm::vec3 getScale() const { return m_transform.scale; }

        /**
         * @brief Returns the model matrix
         **/
        [[nodiscard]] glm::mat4 getModelMatrix() const { return m_transform.modelMatrix(); }

        /**
         * @brief Returns the full transform struct of the model
         **/
        [[nodiscard]] Transform getTransform() const {return m_transform; }

        /**
         * @brief Returns a copy of the CPU-side vertex data
         **/
        [[nodiscard]] std::vector<Vertex> getVertices() const { return m_vertices; }

        /**
        * @brief Returns a copy of the CPU-side index data
        **/
        [[nodiscard]] std::vector<uint32_t> getIndices() const { return m_indices; }
    private:
        friend class MeshManager;
        static Model create(const std::vector<Vertex>& vertices, const BufferLayout& layout);
        static Model create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const BufferLayout& layout);
        static Model load(const std::string& pathObj);
        Model(const std::vector<Vertex>& vertices, const BufferLayout& layout);
        Model(const std::vector<Vertex>& vertices,std::vector<uint32_t> indices, const BufferLayout& layout);
        std::shared_ptr<VertexArray> m_vertexArray;
        BufferLayout m_layout;
        Transform m_transform;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MODEL_H