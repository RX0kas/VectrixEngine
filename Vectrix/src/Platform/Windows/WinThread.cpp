#include "vcpch.h"
#include "Vectrix/Threading/Thread.h"

#include <windows.h>

#include <string>

namespace Vectrix::detail {
	void setPlatformThreadName(const std::string_view name) {
		// SetThreadDescription only exists since Windows 10 1607, and MinGW's headers may not declare it: looked up
		// at run time, and skipped where it's missing (the profiler still gets the name)
		using SetThreadDescriptionFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);
		static const auto setThreadDescription = reinterpret_cast<SetThreadDescriptionFn>(
			reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetThreadDescription")));
		if (!setThreadDescription)
			return;

		const int length = MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
		std::wstring wide(static_cast<size_t>(length), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), wide.data(), length);
		setThreadDescription(GetCurrentThread(), wide.c_str());
	}
}
