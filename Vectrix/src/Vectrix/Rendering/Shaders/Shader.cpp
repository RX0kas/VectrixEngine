#include "Shader.h"

#include "Vectrix/Rendering/Renderer.h"
#include "GraphicAPI/Vulkan/Rendering/Shaders/VulkanShader.h"
#include "Vectrix/Utils/Memory.h"

#include <cctype>
#include <filesystem>
#include <optional>
#include <sstream>

namespace Vectrix {
    std::shared_ptr<Shader> Shader::create(const std::string& name, const std::string& path, const BufferLayout& bufferLayout) {
        return createFromSource(name, readUTF8(path), bufferLayout);
    }

    std::shared_ptr<Shader> Shader::createFromSource(const std::string& name, const std::string& source, const BufferLayout& bufferLayout) {
        bool affectedByCamera;
        const std::optional<ShaderUniformLayout> layout = findShaderUniformLayoutFromSource(name, source, affectedByCamera);
        if (!layout) return nullptr; // Already reported

        switch (Renderer::getAPI()) {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::Vulkan: {
                auto shader = std::make_shared<VulkanShader>(name, source, *layout, bufferLayout, affectedByCamera);
                if (!shader->m_pipeline) {
                    VC_CORE_ERROR_NO_EXIT("Shader {} failed to compile", name);
                    return nullptr;
                }
                return shader;
            }
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
        return nullptr;
    }

    /// @cond INTERNAL
    namespace {
        // The source with its comments turned into spaces, line breaks kept
        std::string stripComments(const std::string& source) {
            std::string result = source;
            for (size_t i = 0; i < result.size(); ++i) {
                if (result.compare(i, 2, "//") == 0) {
                    for (; i < result.size() && result[i] != '\n'; ++i) result[i] = ' ';
                } else if (result.compare(i, 2, "/*") == 0) {
                    const size_t end = result.find("*/", i + 2);
                    const size_t stop = end == std::string::npos ? result.size() : end + 2;
                    for (; i < stop; ++i) if (result[i] != '\n') result[i] = ' ';
                    --i;
                }
            }
            return result;
        }

        // Identifiers and numbers as one token each, any other non-blank character alone
        std::vector<std::string> tokenize(const std::string& source) {
            std::vector<std::string> tokens;
            for (size_t i = 0; i < source.size();) {
                const auto c = static_cast<unsigned char>(source[i]);
                if (std::isspace(c)) { ++i; continue; }
                if (std::isalnum(c) || c == '_') {
                    const size_t start = i;
                    while (i < source.size() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) ++i;
                    tokens.emplace_back(source, start, i - start);
                } else {
                    tokens.emplace_back(1, source[i++]);
                }
            }
            return tokens;
        }

        std::optional<ShaderUniformType> uniformTypeFromGlsl(const std::string& type) {
            if (type == "int")   return ShaderUniformType::Int;
            if (type == "uint")  return ShaderUniformType::Uint;
            if (type == "float") return ShaderUniformType::Float;
            if (type == "bool")  return ShaderUniformType::Bool;
            if (type == "vec2")  return ShaderUniformType::Vec2;
            if (type == "vec3")  return ShaderUniformType::Vec3;
            if (type == "vec4")  return ShaderUniformType::Vec4;
            if (type == "mat4")  return ShaderUniformType::Mat4;
            return std::nullopt;
        }

        struct FrameBlock {
            bool found = false; ///< The stage declares a buffer at set 0, binding 0
            bool valid = true;  ///< Every member has a type the engine can set
            std::vector<std::pair<std::string, ShaderUniformType>> members;
        };

        /*
        Reads the shader's uniforms, the buffer at set 0, binding 0:
        layout(set = 0, binding = 0) readonly buffer FrameSSBO {
            mat4 vc_cameraTransform;
            float time;
        } frame;
        Spaces, line breaks and comments don't matter, the layout's qualifiers can be in any order
        */
        FrameBlock readFrameBlock(const std::string& stageSource, const std::string& shaderName, const char* stage) {
            const std::vector<std::string> tokens = tokenize(stripComments(stageSource));
            FrameBlock block;

            for (size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i] != "layout" || i + 1 >= tokens.size() || tokens[i + 1] != "(") continue;

                // layout(...): set defaults to 0, the binding has to be given
                int set = 0, binding = -1;
                size_t j = i + 2;
                for (; j < tokens.size() && tokens[j] != ")"; ++j) {
                    if (j + 2 < tokens.size() && tokens[j + 1] == "=" && (tokens[j] == "set" || tokens[j] == "binding")) {
                        const int value = std::atoi(tokens[j + 2].c_str());
                        (tokens[j] == "set" ? set : binding) = value;
                        j += 2;
                    }
                }
                // Then the qualifiers up to the block's "{": a buffer has "buffer" among them
                bool isBuffer = false;
                for (++j; j < tokens.size() && tokens[j] != "{" && tokens[j] != ";"; ++j)
                    if (tokens[j] == "buffer") isBuffer = true;
                if (set != 0 || binding != 0 || !isBuffer || j >= tokens.size() || tokens[j] != "{") continue;

                block.found = true;
                // The members, one declaration per ";" up to the "}"
                std::vector<std::string> declaration;
                for (++j; j < tokens.size() && tokens[j] != "}"; ++j) {
                    if (tokens[j] != ";") { declaration.push_back(tokens[j]); continue; }
                    if (declaration.empty()) continue;

                    // Precision qualifiers (highp...) come before the type
                    size_t t = 0;
                    while (t < declaration.size() && (declaration[t] == "highp" || declaration[t] == "mediump" || declaration[t] == "lowp")) ++t;
                    const std::string& typeName = declaration[t < declaration.size() ? t : 0];
                    const auto type = uniformTypeFromGlsl(typeName);

                    // "float a, b;" declares two members, "float a[4];" an array
                    std::vector<std::string> names;
                    bool isArray = false, inBrackets = false;
                    for (size_t n = t + 1; n < declaration.size(); ++n) {
                        if (declaration[n] == "[") isArray = inBrackets = true;
                        else if (declaration[n] == "]") inBrackets = false;
                        else if (!inBrackets && declaration[n] != ",") names.push_back(declaration[n]);
                    }
                    declaration.clear();
                    const std::string firstName = names.empty() ? std::string("?") : names.front();

                    if (!type || isArray) {
                        VC_CORE_ERROR_NO_EXIT("Shader {} ({} stage): the uniform \"{}\" is {}, which can't be set from the engine. "
                                              "The uniforms (set 0, binding 0) can only be int, uint, float, bool, vec2, vec3, vec4 or mat4, not arrays nor structs",
                                              shaderName, stage, firstName, isArray ? std::string("an array") : "a " + typeName);
                        block.valid = false;
                        continue;
                    }
                    for (const auto& name : names) block.members.emplace_back(name, *type);
                }
                return block;
            }
            return block;
        }
    }
    /// @endcond

    std::optional<ShaderUniformLayout> Shader::findShaderUniformLayoutFromSource(const std::string& name, const std::string& source, bool& isAffectedByCamera) {
        isAffectedByCamera = false;
        const auto [vertexSource, fragmentSource] = parseSource(source);
        const FrameBlock vertex = readFrameBlock(vertexSource, name, "vertex");
        const FrameBlock fragment = readFrameBlock(fragmentSource, name, "fragment");
        if (!vertex.valid || !fragment.valid) return std::nullopt;

        // Both stages read the same buffer: the engine writes it with one layout, so they must declare it alike
        if (vertex.found && fragment.found && vertex.members != fragment.members) {
            VC_CORE_ERROR_NO_EXIT("Shader {}: the uniforms (set 0, binding 0) differ between the vertex and the fragment stage. "
                                  "Declare the same members, in the same order and with the same types, in both", name);
            return std::nullopt;
        }

        ShaderUniformLayout layout;
        for (const auto& [memberName, type] : vertex.found ? vertex.members : fragment.members) {
            if (memberName == "vc_cameraTransform") {
                if (type != ShaderUniformType::Mat4) {
                    VC_CORE_ERROR_NO_EXIT("Shader {}: vc_cameraTransform is reserved for the camera and must be a mat4", name);
                    return std::nullopt;
                }
                isAffectedByCamera = true;
            }
            layout.add(memberName, type);
        }
        return layout;
    }

    std::pair<std::string, std::string> Shader::parseSource(const std::string &source) {
        std::istringstream file(source);
        std::string vertex, fragment;
        std::string* current = nullptr;

        std::string line;
        while (std::getline(file, line)) {
            // A stage starts at a line beginning with #shader: a "#shader frag" in a comment or after code isn't one
            const size_t start = line.find_first_not_of(" \t");
            if (start != std::string::npos && line.compare(start, 7, "#shader") == 0) {
                std::string stage;
                std::istringstream(line.substr(start + 7)) >> stage;
                if (stage == "vert") current = &vertex;
                else if (stage == "frag") current = &fragment;
                else {
                    VC_CORE_ERROR_NO_EXIT("Unknown shader stage \"{}\": a .vcshader only has #shader vert and #shader frag", stage);
                    current = nullptr;
                }
                line.clear();
            }
            // Every line of the file is a line of both stages, blank in the one it isn't part of: the line
            // numbers of the compiler's errors are the file's
            vertex += current == &vertex ? line : "";
            vertex += '\n';
            fragment += current == &fragment ? line : "";
            fragment += '\n';
        }

        return {vertex, fragment};
    }
}
