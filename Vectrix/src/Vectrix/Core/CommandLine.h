#ifndef VECTRIXWORKSPACE_COMMANDLINE_H
#define VECTRIXWORKSPACE_COMMANDLINE_H
#include <string>
#include <vector>

/**
 * @file CommandLine.h
 * @brief The command line arguments, in UTF-8 on every platform
 * @ingroup core
 */

namespace Vectrix {
    /**
     * @brief The arguments the application was started with, in UTF-8
     *
     * On Windows, the argv given to main is in the system code page: a file name with a character
     * outside it (e.g. a project opened by double-click from a folder named in another script) arrives
     * with that character replaced, and can't be opened. The arguments are read again from the wide
     * command line there. On Linux they are the bytes main received, which are UTF-8.
     * @param argc The argc given to main
     * @param argv The argv given to main, used as they are where they already are UTF-8 (and on Windows
     *             if the wide command line can't be read)
     * @return Every argument, the program's own name included as the first one
     * @note EntryPoint.h already passes these to createApplication: an application doesn't need to call it
     * @ingroup core
     */
    std::vector<std::string> getCommandLineArguments(int argc, char** argv);
}

#endif //VECTRIXWORKSPACE_COMMANDLINE_H
