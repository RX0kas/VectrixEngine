#include "vcpch.h"
#include "Vectrix/Threading/Thread.h"

#include <pthread.h>

#include <string>

namespace Vectrix::detail {
	void setPlatformThreadName(const std::string_view name) {
		// macOS only names the calling thread, and takes up to 63 bytes
		const std::string shortName(name.substr(0, 63));
		pthread_setname_np(shortName.c_str());
	}
}
