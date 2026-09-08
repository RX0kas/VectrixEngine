#ifndef VECTRIXWORKSPACE_OBJLOADER_H
#define VECTRIXWORKSPACE_OBJLOADER_H

#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <sstream>
#include "Vertex.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Rendering/Buffer.h"
#include "Vectrix/Utils/Hashing.h"

/**
 * @file ObjLoader.h
 * @brief Reading of a Wavefront OBJ file into vertices and indices
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief Return the vertex layout an OBJ file is loaded into
     *
     * Position, normal and texture coordinates, in that order, which is what the shaders
     * drawing a loaded model expect.
     * @return The layout matching what loadOBJ produces
     * @see loadOBJ
     * @ingroup mesh
     */
    inline BufferLayout getTinyObjLayout() {
        return {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        };
    }

    /**
     * @brief Read a Wavefront OBJ file into vertices and indices
     *
     * Vertices repeated across faces are merged, so the same position, normal and texture
     * coordinate triple is only stored once and referred to by index. A missing normal or
     * texture coordinate becomes zero rather than failing the load.
     * @param filepath The path of the OBJ file to read
     * @param outVertices Receives the vertices, appended to what is already there
     * @param outIndices Receives the indices, appended to what is already there
     * @return true when the file could be read
     * @see getTinyObjLayout
     * @ingroup mesh
     */
    inline bool loadOBJ(const std::string& filepath,std::vector<Vertex>& outVertices,std::vector<uint32_t>& outIndices) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;

        const bool ret = tinyobj::LoadObj(&attrib,&shapes,&materials,&err,filepath.c_str());


        if (!err.empty()) {
            VC_ERROR("OBJ error: {}",err);
            return false;
        }

        if (!ret) {
            return false;
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                // POSITION
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // NORMAL
                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                } else {
                    vertex.normal = glm::vec3(0.0f);
                }

                // TEXCOORD
                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                } else {
                    vertex.uv = glm::vec2(0.0f);
                }

                // DEDUPLICATION
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(outVertices.size());
                    outVertices.push_back(vertex);
                }

                outIndices.push_back(uniqueVertices[vertex]);
            }
        }

        return true;
    }
}

#endif //VECTRIXWORKSPACE_OBJLOADER_H