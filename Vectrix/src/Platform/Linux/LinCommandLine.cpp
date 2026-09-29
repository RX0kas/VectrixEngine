#include "vcpch.h"
#include "Vectrix/Core/CommandLine.h"

namespace Vectrix {
	std::vector<std::string> getCommandLineArguments(int argc, char** argv) {
		// Linux passes file names as the bytes they are stored with, which are UTF-8
		return {argv, argv + argc};
	}
}
