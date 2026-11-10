#include "vcpch.h"
#include "Vectrix/Threading/Thread.h"

#include <pthread.h>

#include <string>

namespace Vectrix::detail {
	void setPlatformThreadName(const std::string_view name) {
		// The kernel refuses a name over 15 bytes (16 with the terminator) instead of cutting it
		const std::string shortName(name.substr(0, 15));
		pthread_setname_np(pthread_self(), shortName.c_str());
	}
}
