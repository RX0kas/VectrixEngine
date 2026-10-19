#include "vcpch.h"
#include "Vectrix/Core/CommandLine.h"

namespace Vectrix {
	std::vector<std::string> getCommandLineArguments(int argc, char** argv) {
		return {argv, argv + argc};
	}
}