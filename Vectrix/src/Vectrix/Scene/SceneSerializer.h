#ifndef VECTRIXWORKSPACE_SCENESERIALIZER_H
#define VECTRIXWORKSPACE_SCENESERIALIZER_H
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <optional>

#include "Vectrix/Core/AppInfo.h"

#define SCENE_VERSION_MAJOR 0
#define SCENE_VERSION_MINOR 1
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

    struct ComponentCreationData {
        uint32_t type_id;
        std::vector<std::byte> data;
    };

    struct EntityCreationData {
        std::string name;
        std::vector<ComponentCreationData> components;
    };

    struct SceneCreationData {
        bool loaded = false;
        uint32_t engine_version;
        uint32_t file_version;
        std::string name;
        std::vector<EntityCreationData> entities;
    };
    class SceneSerializer {
    public:
        static SceneCreationData loadSceneFile(const std::string& path);
    private:
        static bool validMagicNumber(std::ifstream& stream);
        static std::optional<uint32_t> validVectrixVersion(std::ifstream& stream);
        static std::optional<uint32_t> validSceneVersion(std::ifstream& stream);
        static std::optional<std::string> getName(std::ifstream& stream);
        static std::optional<std::vector<EntityCreationData>> readEntities(std::ifstream& stream);
        static std::optional<std::vector<ComponentCreationData>> readComponents(std::ifstream& stream);

        static constexpr std::uint32_t MAGIC_NUMBER = 0x56435458; // VCTX
        static constexpr Version SCENE_VERSION = static_cast<uint32_t>(VC_PLATFORM_ID)<< 31U | static_cast<uint32_t>(SCENE_VERSION_MAJOR) << 24U | static_cast<uint32_t>(SCENE_VERSION_MINOR) << 12U | static_cast<uint32_t>(SCENE_VERSION_PATCH);
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENESERIALIZER_H