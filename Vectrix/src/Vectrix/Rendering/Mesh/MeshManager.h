#ifndef VECTRIXWORKSPACE_MESH_MANAGER_H
#define VECTRIXWORKSPACE_MESH_MANAGER_H
#include "Model.h"
#include "ObjLoader.h"
#include "Vectrix/Core/Core.h"

#include "Vectrix/Utils/Hashing.h"
#include "Vectrix/Utils/Memory.h"

/**
 * @file MeshManager.h
 * @brief Definition of the class that manage the life of mesh
 */

namespace Vectrix {
    /**
     * @brief Manages Mesh
     */
    class MeshManager {
    public:


        /**
         * @brief This function is used to get a mesh previously created
         * @pre A mesh with this name must exist
         * @param name Name of the mesh you wanna get
         * @return The mesh program with the name you entered
         */
        std::shared_ptr<Model> get(const std::string& name);

        /**
         * @brief This function is used to get all the mesh previously created
         * @return The mesh programs created
         */
        std::vector<std::shared_ptr<Model>> getAll() {
            return m_cache.getAllItems();
        }

        /**
         * @brief This function is used to get all the mesh previously created with their names
         * @return The mesh programs created
         */
        std::vector<std::pair<std::string,std::shared_ptr<Model>>> getAllWithName() {
            return m_cache.getAll();
        }

        /**
         * @brief Return a reference to the instance of this class
         * @return The instance of the class MeshManager
         */
        static MeshManager& instance() { return *s_instance; }

        /**
         * @brief Function to load a new mesh
         * @param name Name given to the mesh
         * @param path Path of the mesh file (OBJ file)
         * @return A reference to the model that was loaded
         */
        static std::shared_ptr<Model> loadModel(const std::string &name, const std::string &path);

        /**
         * @brief Function to create a new mesh
         * @param name Name given to the mesh
         * @param vertices
         * @param indices
         * @return A reference to the model that was created
         */
        static std::shared_ptr<Model> createModel(const std::string &name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const BufferLayout& layout=getTinyObjLayout());
        /**
         * @brief Function to create a new mesh
         * @param name Name given to the mesh
         * @param vertices
         * @return A reference to the model that was created
         */
        static std::shared_ptr<Model> createModel(const std::string &name, const std::vector<Vertex>& vertices, const BufferLayout& layout=getTinyObjLayout());

        ~MeshManager();
    private:
        friend class Model;
        friend class Application;
        MeshManager();
        bool remove(const std::string& name);
        void add(const std::string& name,std::shared_ptr<Model> mesh);
        Cache<std::string,std::shared_ptr<Model>> m_cache;
        std::shared_ptr<Model> m_notFoundMesh;

    private:
        static MeshManager* s_instance;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESH_MANAGER_H