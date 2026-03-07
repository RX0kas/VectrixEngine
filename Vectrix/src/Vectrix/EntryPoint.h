#pragma once

#if defined(VC_PLATFORM_WINDOWS) || defined(VC_PLATFORM_LINUX)


extern Vectrix::Application* Vectrix::createApplication();

int main(int argc, char** argv)
{
	Vectrix::Log::init();
	VC_PROFILER_BEGIN_SESSION("Startup","Profile-Startup.json");
#ifdef VC_PLATFORM_WINDOWS
	VC_CORE_WARN("The support for Windows is still experimental");
#endif
	auto app = Vectrix::createApplication();
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
