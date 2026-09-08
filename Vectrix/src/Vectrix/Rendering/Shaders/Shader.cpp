#include "Shader.h"

#include "Vectrix/Rendering/Renderer.h"
#include "GraphicAPI/Vulkan/Rendering/Shaders/VulkanShader.h"

#include <filesystem>

namespace Vectrix {
    std::shared_ptr<Shader> Shader::create(const std::string& name, const std::string& path, const BufferLayout& bufferLayout) {
        bool affectedByCamera;
        ShaderUniformLayout layout = findShaderUniformLayout(path,affectedByCamera);

        switch (Renderer::getAPI()) {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return std::make_shared<VulkanShader>(name, path, layout, bufferLayout, affectedByCamera);
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
        return nullptr;
    }

    /// @cond INTERNAL
    void replaceAll(std::string& str, const std::string& from, std::string to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
    /// @endcond

    /// @cond INTERNAL
    std::string normalizeSpaces(const std::string& s) {
        std::string result;
        bool lastWasSpace = false;
        for (char c : s) {
            if (c == ' ' || c == '\t') {
                if (!lastWasSpace) result += ' ';
                lastWasSpace = true;
            } else {
                result += c;
                lastWasSpace = false;
            }
        }
        return result;
    }
    /// @endcond

    /*
    layout(set = 0, binding = 0) readonly buffer FrameSSBO {
    mat4 vc_cameraTransform;
    float time;
    } frame;
    */
    ShaderUniformLayout Shader::findShaderUniformLayout(const std::string &path, bool &isAffectedByCamera) {
        std::filesystem::path p(path);
        if (p.empty() || !std::filesystem::exists(p)) { VC_CORE_ERROR("Shader {} does not exist", path); }

        std::ifstream file(p);
        ShaderUniformLayout layout;
        std::string line;
        std::stringstream contentStream;
        bool isContent = false;
        isAffectedByCamera = false;

        while (std::getline(file, line)) {
            std::string lineCopy = normalizeSpaces(line);
            char* l = static_cast<char*>(malloc(lineCopy.size()));
            lineCopy.copy(l,lineCopy.size());
            std::string lineWithOutSpace = { l };
            replaceAll(lineWithOutSpace," ","");
            // Trim leading space
            if (!lineCopy.empty() && lineCopy.front() == ' ')
                lineCopy = lineCopy.substr(1);

            if (lineWithOutSpace.find("layout(set=0,binding=0)readonlybuffer") != std::string::npos) {
                isContent = true;
                continue;
            }
            free(l);


            if (isContent && lineCopy.find('{') != std::string::npos) {
                continue;
            }
            if (isContent && lineCopy.find('}') != std::string::npos) {
                break;
            }
            if (isContent) {
                if (lineCopy.find("vc_cameraTransform") != std::string::npos)
                    isAffectedByCamera = true;
                contentStream << lineCopy << ' ';
            }
        }

        std::string content = contentStream.str();
        std::stringstream layoutStream;
        ShaderUniformType type = ShaderUniformType::Float;
        bool lastWasSpace = false;

        for (char c : content) {
            if (c == ';') {
                std::string name = layoutStream.str();
                replaceAll(name, " ", "");
                if (!name.empty()) {
                    layout.add(name, type);
                }
                layoutStream.str("");
                layoutStream.clear();
                lastWasSpace = false;
                continue;
            }

            if (c == ' ' && !lastWasSpace) {
                std::string t = layoutStream.str();
                replaceAll(t, " ", "");
                if (t == "int") {
                    type = ShaderUniformType::Int;
                }
                else if (t == "float") {
                    type = ShaderUniformType::Float;
                }
                else if (t == "vec2") {
                    type = ShaderUniformType::Vec2;
                }
                else if (t == "vec3") {
                    type = ShaderUniformType::Vec3;
                }
                else if (t == "vec4") {
                    type = ShaderUniformType::Vec4;
                }
                else if (t == "mat4") {
                    type = ShaderUniformType::Mat4;
                }
                else if (t == "uint") {
                    type = ShaderUniformType::Uint;
                }
                else if (t == "bool") {
                    type = ShaderUniformType::Bool;
                }
                else if (!t.empty()) {
                    VC_CORE_ERROR("Unknown type {}", t);
                }
                layoutStream.str("");
                layoutStream.clear();
                lastWasSpace = true;
                continue;
            }

            if (c == ' ' && lastWasSpace)
                continue;

            lastWasSpace = false;
            layoutStream << c;
        }

        return layout;
    }

    std::pair<std::string, std::string> Shader::parse(const std::string &path) {
        std::filesystem::path p = std::filesystem::path(path);
        if (p.empty() || !std::filesystem::exists(p)) { VC_CORE_ERROR("Shader {} does not exist",path); }
        std::ifstream file(p);
        std::stringstream vertex, fragment;
        std::stringstream* current = nullptr;

        std::string line;
        while (std::getline(file, line)) {
            if (line.find("#shader vert") != std::string::npos)
                current = &vertex;
            else if (line.find("#shader frag") != std::string::npos)
                current = &fragment;
            else if (current)
                *current << line << '\n';
        }

        return {vertex.str(), fragment.str()};
    }
}
