#ifndef VECTRIXWORKSPACE_SCENESERIALIZER_H
#define VECTRIXWORKSPACE_SCENESERIALIZER_H
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <optional>

#include "Scene.h"
#include "Scene.h"
#include "Vectrix/Core/AppInfo.h"
#include "Vectrix/Utils/Result.h"

/**
 * @file SceneSerializer.h
 * @brief Reading and writing of the binary scene files
 * @ingroup ecs
 */

/**
 * @brief The major part of the scene file format version, bumped by a breaking change
 * @ingroup ecs
 */
#define SCENE_VERSION_MAJOR 0

/**
 * @brief The minor part of the scene file format version, bumped by an addition
 * @ingroup ecs
 */
#define SCENE_VERSION_MINOR 1

/**
 * @brief The patch part of the scene file format version, bumped by a fix
 * @ingroup ecs
 */
#define SCENE_VERSION_PATCH 0

namespace Vectrix {
    /*
    [HEADER]
        uint32_t  magic;
        uint32_t  engine_version;
        uint32_t  file_version;
        uint16_t  scene_name_len;
        char[]    scene_name;
        uint32_t  entity_count;

    [ENTITIES] × entity_count
        uint16_t  name_len;
        char[]    name;
        uint16_t  component_count;

    [COMPONENTS] × component_count
        uint32_t  type_id;
        uint32_t  data_size;
        bytes[]   data;

    [FOOTER]
        uint32_t  hash;
     */
    class Scene;

    /**
     * @brief One component read from a scene file, still in its raw form
     *
     * The bytes are only turned into an actual component once the entity owning it is
     * built, which is why the type is kept as an identifier rather than a real type.
     * @see EntityCreationData
     * @ingroup ecs
     */
    struct ComponentCreationData {
        /// Identifies which component the bytes describe
        uint32_t type_id;

        /// The component content, as stored in the file
        std::vector<std::byte> data;
    };

    /**
     * @brief One entity read from a scene file, with the components it owns
     * @see SceneCreationData
     * @ingroup ecs
     */
    struct EntityCreationData {
        /// The name the entity was saved under
        std::string name;

        /// Every component the entity owns, still in its raw form
        std::vector<ComponentCreationData> components;
    };

    /**
     * @brief Everything a scene file contains, once it has been read
     *
     * Check #result before using any other field: when the file could not be read the
     * rest is meaningless.
     * @see SceneSerializer::loadSceneFile
     * @see Scene::loadScene
     * @ingroup ecs
     */
    struct SceneCreationData {
        /// Tells whether the file could be read, and what went wrong otherwise
        VectrixResult result;

        /// The version of the engine that wrote the file
        uint32_t engine_version;

        /// The version of the scene format the file uses
        uint32_t file_version;

        /// The name of the scene
        std::string name;

        /// Every entity of the scene, in the order they were saved
        std::vector<EntityCreationData> entities;
    };

    /**
     * @brief Writes a Scene to a file, and reads one back
     *
     * The format is binary: a header carrying the magic number and the versions, then the
     * entities with their components, then a hash of the content used to detect a
     * corrupted file. Loading only produces a SceneCreationData, building the actual
     * Scene from it is Scene::loadScene's job.
     * @see Scene
     * @ingroup ecs
     */
    class SceneSerializer {
    public:
        /**
         * @brief Read a scene file and return what it contains
         * @param path The path of the file to read
         * @return The content of the file, its `result` field telling whether it worked
         * @note Nothing is created here, the returned data still has to be turned into a Scene
         * @see Scene::loadScene
         */
        static SceneCreationData loadSceneFile(const std::string& path);

        /**
         * @brief Write a scene to a file, replacing it when it is already there
         * @param path The path of the file to write
         * @param scene The scene to save
         * @return Whether the scene could be written
         */
        static VectrixResult saveScene(const std::string& path, Scene& scene);
    private:
        static bool validMagicNumber(std::ifstream& stream);
        static std::optional<uint32_t> validVectrixVersion(std::ifstream& stream);
        static std::optional<uint32_t> validSceneVersion(std::ifstream& stream);
        static std::optional<std::string> getString(std::ifstream& stream);
        static std::optional<std::vector<EntityCreationData>> readEntities(std::ifstream& stream);
        static std::optional<std::vector<ComponentCreationData>> readComponents(std::ifstream& stream);

        static constexpr std::uint32_t MAGIC_NUMBER = 0x56435458; // VCTX
        static constexpr Version SCENE_VERSION = static_cast<uint32_t>(VC_PLATFORM_ID)<< 31U | static_cast<uint32_t>(SCENE_VERSION_MAJOR) << 24U | static_cast<uint32_t>(SCENE_VERSION_MINOR) << 12U | static_cast<uint32_t>(SCENE_VERSION_PATCH);
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENESERIALIZER_H
