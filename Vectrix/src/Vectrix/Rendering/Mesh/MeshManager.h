#ifndef VECTRIXWORKSPACE_MESHMANAGER_H
#define VECTRIXWORKSPACE_MESHMANAGER_H
#include <memory>

#include "Mesh.h"
#include "Vectrix/Utils/Memory.h"

/**
 * @file MeshManager.h
 * @brief Definition of the MeshManager class
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief Keeps the loaded meshes, so the same model is only uploaded once
     *
     * The AssetsManager owns it and goes through it to load a mesh, reach it with
     * AssetsManager::getMeshManager.
     * @see AssetsManager
     * @see Mesh
     * @ingroup mesh
     */
    class MeshManager {
    public:
            /**
             * @brief This function is used to get a mesh previously created
             * @pre A mesh with this name must exist
             * @param name Name of the mesh you wanna get
             * @return The mesh with the name you entered
             */
            std::shared_ptr<Mesh> get(const std::string& name);

            /**
             * @brief This function is used to know if a mesh with the name given exist
             * @param name Name of the mesh
             * @return True if the mesh exist in the cache
             */
            bool exist(const std::string& name) {
                return m_cache.find(name) != m_cache.end();
            }

            /**
             * @brief This function is used to get all the mesh previously created
             */
            std::vector<std::shared_ptr<Mesh>> getAll() {
                return m_cache.getAllItems();
            }

            /**
             * @brief Function to create a new mesh
             * @pre A mesh with the name given should not already exist
             * @param name Name given to the mesh
             * @param path Path of the mesh source file
             */
            std::shared_ptr<Mesh> createMesh(const std::string &name, const std::string &path);
            ~MeshManager();
        private:
            friend class Mesh;
            friend class Scene;
            MeshManager();
            friend class AssetsManager;
            friend class Application;
            friend class EditorLayer;
            bool remove(const std::string& name);
            void add(const std::string& name,std::shared_ptr<Mesh> mesh);
            void clear();
            Cache<std::string,std::shared_ptr<Mesh>> m_cache;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHMANAGER_H