#pragma once

#if defined(VC_PLATFORM_WINDOWS) || defined(VC_PLATFORM_LINUX)


extern Vectrix::Application* Vectrix::createApplication();

int main(int argc, char** argv)
{
	Vectrix::Log::init();
#ifdef VC_PLATFORM_WINDOWS
	VC_CORE_WARN("The support for Windows is still experimental");
#endif


	auto app = Vectrix::createApplication();
	app->run();
	

	delete app;
}

#else
#error Vectrix only supports windows and Linux
#endif // VC_PLATFORM_WINDOWS
