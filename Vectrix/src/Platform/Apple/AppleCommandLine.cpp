#include "vcpch.h"
#include "Vectrix/Core/CommandLine.h"

#include <mach-o/dyld.h> // _NSGetExecutablePath

namespace Vectrix {
	std::vector<std::string> getCommandLineArguments(int argc, char** argv) {
		// macOS passes file names in UTF-8
		return {argv, argv + argc};
	}

	std::filesystem::path getExecutablePath() {
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size); // gives the size needed
		std::string path(size, '\0');
		if (_NSGetExecutablePath(path.data(), &size) != 0)
			return {};
		path.resize(std::strlen(path.c_str()));
		// The path may go through symbolic links or "..": resolved, so its parent is the real folder
		std::error_code ec;
		std::filesystem::path resolved = std::filesystem::weakly_canonical(std::filesystem::path(path), ec);
		return ec ? std::filesystem::path(path) : resolved;
	}
}