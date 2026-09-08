#pragma once

/**
 * @file EntryPoint.h
 * @brief The `main` function of every application built on the engine
 * @ingroup core
 *
 * Include this header in exactly one translation unit of the application. It defines
 * `main`, which builds the Application through Vectrix::createApplication, runs it and
 * profiles the three phases of its life.
 * @see Vectrix::createApplication
 */

#if defined(VC_PLATFORM_WINDOWS) || defined(VC_PLATFORM_LINUX)


extern Vectrix::Application* Vectrix::createApplication();

/**
 * @brief Start the engine, run the application, then tear it down
 *
 * The application has to provide Vectrix::createApplication, which returns the instance
 * that gets run here. Startup, runtime and shutdown are each profiled into their own
 * session file.
 * @param argc The number of command line arguments
 * @param argv The command line arguments
 * @return 0 once the application stopped
 * @ingroup core
 */
int main(int argc, char** argv) {
	VC_PROFILER_BEGIN_SESSION("Startup","Profile-Startup.json");
	Vectrix::Log::init();
	const auto app = Vectrix::createApplication();
	VC_PROFILER_END_SESSION();

	VC_PROFILER_BEGIN_SESSION("Runtime","Profile-Runtime.json");
	app->run();
	VC_PROFILER_END_SESSION();
	
	VC_PROFILER_BEGIN_SESSION("Shutdown","Profile-Shutdown.json");
	delete app;
	VC_PROFILER_END_SESSION();
}

#else
#error Vectrix only supports windows and Linux
#endif // VC_PLATFORM_WINDOWS
