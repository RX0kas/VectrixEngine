#ifndef VECTRIXWORKSPACE_MESHMANAGER_H
#define VECTRIXWORKSPACE_MESHMANAGER_H
#include <memory>

#include "Mesh.h"
#include "Vectrix/Utils/Memory.h"

namespace Vectrix {
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
            MeshManager();
            friend class AssetsManager;
            bool remove(const std::string& name);
            void add(const std::string& name,std::shared_ptr<Mesh> mesh);
            Cache<std::string,std::shared_ptr<Mesh>> m_cache;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHMANAGER_H