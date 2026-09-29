#ifndef VECTRIXWORKSPACE_PATH_H
#define VECTRIXWORKSPACE_PATH_H
#include <filesystem>
#include <string>
#include <string_view>

/**
 * @file Path.h
 * @brief Conversions between paths and the UTF-8 strings Vectrix keeps them in
 *
 * A path held in a std::string (settings, scene and project files, asset ids, ImGui text, file
 * dialogs, logs) is UTF-8. std::filesystem::path::string() and building a path from a std::string
 * use the system code page instead on Windows: accented folder names come out garbled there, and
 * characters the code page can't represent make string() throw. These go through UTF-8 on every
 * platform, and are plain copies on Linux, where the native encoding already is UTF-8.
 */

namespace Vectrix {
    /**
     * @brief The path as a UTF-8 string, with the platform's separators
     * @ingroup tools
     */
    inline std::string toUtf8(const std::filesystem::path& path) {
        const std::u8string text = path.u8string();
        return {text.begin(), text.end()};
    }

    /**
     * @brief The path as a UTF-8 string with '/' separators, for what is written to files read on other machines
     * @ingroup tools
     */
    inline std::string toGenericUtf8(const std::filesystem::path& path) {
        const std::u8string text = path.generic_u8string();
        return {text.begin(), text.end()};
    }

    /**
     * @brief The path a UTF-8 string names
     * @ingroup tools
     */
    inline std::filesystem::path fromUtf8(std::string_view utf8) {
        return {std::u8string(utf8.begin(), utf8.end())};
    }
}

#endif //VECTRIXWORKSPACE_PATH_H
