// Runs every VC_TEST, or those whose name contains the first argument. Exit code 0 when all pass
#include "Test.h"

#include <cstring>
#include <exception>

#include "Vectrix/Application.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Threading/Thread.h"

VC_SET_APP_INFO("VectrixTests", 0, 0, 0)

int main(int argc, char** argv) {
	Vectrix::detail::setMainThread();
	Vectrix::Log::init();
	const char* filter = argc > 1 ? argv[1] : nullptr;

	int run = 0;
	int failedTests = 0;
	for (const VectrixTests::TestCase& test : VectrixTests::registry()) {
		if (filter && !std::strstr(test.name, filter))
			continue;
		++run;
		const int failuresBefore = VectrixTests::failureCount();
		std::printf("[ RUN  ] %s\n", test.name);
		std::fflush(stdout);
		try {
			test.body();
		} catch (const VectrixTests::RequireFailed&) {
			// Already reported
		} catch (const std::exception& exception) {
			VectrixTests::reportFailure(exception.what(), "uncaught exception", 0);
		} catch (...) {
			VectrixTests::reportFailure("unknown exception", "uncaught exception", 0);
		}
		const bool passed = VectrixTests::failureCount() == failuresBefore;
		failedTests += passed ? 0 : 1;
		std::printf("[ %s ] %s\n", passed ? " OK " : "FAIL", test.name);
	}
	std::printf("%d test(s), %d failed\n", run, failedTests);
	return failedTests == 0 && run > 0 ? 0 : 1;
}
