#pragma once

/**
 * @file EntryPoint.h
 * @brief The `main` function of every application built on the engine
 * @ingroup core
 *
 * Include this header in exactly one translation unit of the application. It defines
 * `main`, which builds the Application through Vectrix::createApplication, runs it and
 * profiles the three phases of its life when asked (`--vc-profile`, see Vectrix::EngineOptions).
 * @see Vectrix::createApplication
 */

#if defined(VC_PLATFORM_WINDOWS) || defined(VC_PLATFORM_LINUX) || defined(VC_PLATFORM_APPLE)

#include <filesystem>
#include <string>
#include <vector>

#include "Vectrix/Core/CommandLine.h"
#include "Vectrix/Core/EngineOptions.h"
#include "Vectrix/Threading/Thread.h"
#include "Vectrix/Utils/Path.h"

extern Vectrix::Application* Vectrix::createApplication(int argc, char** argv);

/**
 * @brief Start the engine, run the application, then tear it down
 *
 * The application has to provide Vectrix::createApplication, which returns the instance
 * that gets run here. The engine's options (`--vc-...`, see EngineOptions) are taken out of the
 * arguments first. With `--vc-profile`, startup, runtime and shutdown are each profiled into their own
 * session file.
 * @param argc The number of command line arguments
 * @param argv The command line arguments, e.g. a project/scene file path passed by the OS
 *             when the application is launched via a file association. createApplication gets
 *             them in UTF-8 (see Vectrix::getCommandLineArguments)
 * @return 0 once the application stopped
 * @ingroup core
 */
int main(int argc, char** argv) {
	Vectrix::detail::setMainThread();
	Vectrix::Log::init();
	// Alive until main returns, as the application may keep argv's pointers
	std::vector<std::string> arguments = Vectrix::getCommandLineArguments(argc, argv);
	// The engine's own --vc-... options are taken out: the application only gets its own arguments
	const Vectrix::EngineOptions options = Vectrix::extractEngineOptions(arguments);
	[[maybe_unused]] const std::filesystem::path profileFolder = options.profile ? options.profileOutputFolder() : std::filesystem::path{};
	if (options.profile) {
		VC_PROFILER_BEGIN_SESSION("Startup", profileFolder / "Profile-Startup.json");
	}
	std::vector<char*> utf8Argv;
	utf8Argv.reserve(arguments.size() + 1);
	for (std::string& argument : arguments)
		utf8Argv.push_back(argument.data());
	utf8Argv.push_back(nullptr); // argv[argc] is null, as main's
	const auto app = Vectrix::createApplication(static_cast<int>(arguments.size()), utf8Argv.data());
	if (options.profile) {
		VC_PROFILER_END_SESSION();
		VC_PROFILER_BEGIN_SESSION("Runtime", profileFolder / "Profile-Runtime.json");
	}
	app->run();
	if (options.profile) {
		VC_PROFILER_END_SESSION();
		VC_PROFILER_BEGIN_SESSION("Shutdown", profileFolder / "Profile-Shutdown.json");
	}
	delete app;
	if (options.profile) {
		VC_PROFILER_END_SESSION();
		VC_CORE_INFO("Profile files written to {}", Vectrix::toUtf8(profileFolder));
	}
	return 0;
}

#else
#error Vectrix only supports Windows, Linux and macOS
#endif // VC_PLATFORM_WINDOWS
