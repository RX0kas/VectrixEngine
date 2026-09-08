#ifndef VECTRIXWORKSPACE_OUTLINE_H
#define VECTRIXWORKSPACE_OUTLINE_H
#include <glm/glm.hpp>

#include "Vectrix/Utils/Json.h"

/**
 * @file Outline.h
 * @brief Runtime settings for the selection outline pass, and their JSON bridge
 * @ingroup settings
 */

namespace Vectrix {
    /**
     * @brief Tunables the outline pass reads every time it draws
     */
    struct OutlineSettings {
        glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        float thickness = 0.1f;
        glm::vec2 texelSize = {1.0f, 1.0f};
    };

    /**
     * @brief The live outline settings the renderer samples
     *
     * The editor edits this directly for immediate feedback and mirrors it to the
     * project settings file through readOutlineSettings() / writeOutlineSettings().
     */
    inline OutlineSettings outlineSettings;

    /**
     * @brief Copy outline fields out of a JSON node into #outlineSettings.
     * @param node A node shaped like { "color": [r,g,b,a], "thickness": t, "texelSize": [x,y] }.
     *             Missing or wrongly typed fields keep their current value.
     */
    inline void readOutlineSettings(const JsonValue& node) {
        if (!node.isType<JsonObject>())
            return;

        if (node.contains("thickness"))
            outlineSettings.thickness = static_cast<float>(node["thickness"].getDouble());

        if (node.contains("color")) {
            const JsonValue& color = node["color"];
            for (size_t i = 0; i < color.size() && i < 4; ++i)
                outlineSettings.color[static_cast<glm::length_t>(i)] = static_cast<float>(color[i].getDouble());
        }

        if (node.contains("texelSize")) {
            const JsonValue& texel = node["texelSize"];
            for (size_t i = 0; i < texel.size() && i < 2; ++i)
                outlineSettings.texelSize[static_cast<glm::length_t>(i)] = static_cast<float>(texel[i].getDouble());
        }
    }

    /**
     * @brief Write the current #outlineSettings into a JSON node, ready to be serialized.
     * @param node The node to fill; it becomes an object with "color", "thickness" and
     *             "texelSize" members.
     */
    inline void writeOutlineSettings(JsonValue& node) {
        node["thickness"] = static_cast<double>(outlineSettings.thickness);

        JsonArray color;
        for (int i = 0; i < 4; ++i)
            color.emplace_back(static_cast<double>(outlineSettings.color[i]));
        node["color"] = color;

        JsonArray texel;
        for (int i = 0; i < 2; ++i)
            texel.emplace_back(static_cast<double>(outlineSettings.texelSize[i]));
        node["texelSize"] = texel;
    }
}

#endif //VECTRIXWORKSPACE_OUTLINE_H
