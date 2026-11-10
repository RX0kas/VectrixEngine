#pragma once

// A minimal test harness, so the engine's tests need no dependency: VC_TEST registers a test, VC_CHECK reports a
// failed condition without stopping the test, VC_REQUIRE stops it

#include <cstdio>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace VectrixTests {
	struct TestCase {
		const char* name;
		std::function<void()> body;
	};

	inline std::vector<TestCase>& registry() {
		static std::vector<TestCase> tests;
		return tests;
	}

	inline int& failureCount() {
		static int failures = 0;
		return failures;
	}

	struct Registrar {
		Registrar(const char* name, std::function<void()> body) { registry().push_back({name, std::move(body)}); }
	};

	struct RequireFailed : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	inline void reportFailure(const char* expression, const char* file, int line) {
		++failureCount();
		std::fprintf(stderr, "  FAILED %s (%s:%d)\n", expression, file, line);
	}
}

#define VC_TEST_CONCAT_INNER(a, b) a##b
#define VC_TEST_CONCAT(a, b) VC_TEST_CONCAT_INNER(a, b)

#define VC_TEST(name) \
	static void VC_TEST_CONCAT(vcTest_, __LINE__)(); \
	static const ::VectrixTests::Registrar VC_TEST_CONCAT(vcTestRegistrar_, __LINE__)(name, &VC_TEST_CONCAT(vcTest_, __LINE__)); \
	static void VC_TEST_CONCAT(vcTest_, __LINE__)()

#define VC_CHECK(condition) \
	do { if (!(condition)) ::VectrixTests::reportFailure(#condition, __FILE__, __LINE__); } while (0)

#define VC_REQUIRE(condition) \
	do { if (!(condition)) { ::VectrixTests::reportFailure(#condition, __FILE__, __LINE__); throw ::VectrixTests::RequireFailed(#condition); } } while (0)
