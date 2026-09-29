#include "vcpch.h"
#include "Vectrix/Core/CommandLine.h"

#include <shellapi.h> // CommandLineToArgvW, left out by WIN32_LEAN_AND_MEAN

namespace Vectrix {
	std::vector<std::string> getCommandLineArguments(int argc, char** argv) {
		// main's argv went through the code page: the wide command line is the one with every character
		int count = 0;
		LPWSTR* wide = CommandLineToArgvW(GetCommandLineW(), &count);
		if (wide == nullptr)
			return {argv, argv + argc};

		std::vector<std::string> arguments;
		arguments.reserve(static_cast<size_t>(count));
		for (int i = 0; i < count; ++i) {
			// The size includes the terminating null, which std::string keeps on its own
			const int size = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
			std::string argument(size > 1 ? static_cast<size_t>(size - 1) : 0, '\0');
			if (size > 1)
				WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, argument.data(), size, nullptr, nullptr);
			arguments.push_back(std::move(argument));
		}
		LocalFree(wide);
		return arguments;
	}
}
