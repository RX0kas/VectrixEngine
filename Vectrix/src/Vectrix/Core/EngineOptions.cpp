#include "vcpch.h"
#include "EngineOptions.h"

#include <charconv>
#include <cstdlib>
#include <string_view>

#include "Vectrix/Utils/Folders.h"
#include "Vectrix/Utils/Path.h"

namespace Vectrix {
	namespace {
		EngineOptions s_options;

		constexpr std::string_view k_prefix = "--vc-";

		std::optional<std::uint64_t> parseCount(std::string_view text) {
			std::uint64_t value = 0;
			const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
			if (error != std::errc() || end != text.data() + text.size())
				return std::nullopt;
			return value;
		}
	}

	std::filesystem::path EngineOptions::profileOutputFolder() const {
		if (!profileFolder.empty())
			return profileFolder;
		return getVectrixStateFolder() / fromUtf8(Application::getAppInfo().getAppName()) / "profiles";
	}

	EngineOptions extractEngineOptions(std::vector<std::string>& arguments) {
		EngineOptions options;

		// VC_PROFILE=1 turns the profiler on, any other value but 0 is the folder to write to
		if (const char* env = std::getenv("VC_PROFILE"); env && *env && std::string_view(env) != "0") {
			options.profile = true;
			if (std::string_view(env) != "1")
				options.profileFolder = fromUtf8(env);
		}

		// argv[0] is the program: never an option
		for (size_t i = 1; i < arguments.size();) {
			const std::string_view argument = arguments[i];
			if (!argument.starts_with(k_prefix)) {
				++i;
				continue;
			}
			const size_t equal = argument.find('=');
			const std::string_view name = argument.substr(k_prefix.size(), equal == std::string_view::npos ? std::string_view::npos : equal - k_prefix.size());
			std::optional<std::string> value;
			if (equal != std::string_view::npos)
				value = std::string(argument.substr(equal + 1));
			size_t consumed = 1;

			if (name == "profile") {
				options.profile = true;
				if (value && !value->empty())
					options.profileFolder = fromUtf8(*value);
			} else if (name == "frames") {
				// The count may also be the next argument: --vc-frames 100
				if (!value && i + 1 < arguments.size()) {
					value = arguments[i + 1];
					consumed = 2;
				}
				const std::optional<std::uint64_t> count = value ? parseCount(*value) : std::nullopt;
				if (count && *count > 0)
					options.frameLimit = count;
				else
					VC_CORE_WARN("--vc-frames needs a number of frames above 0, ignored ('{}')", value.value_or(""));
			} else {
				VC_CORE_WARN("Unknown engine option '{}', ignored", argument);
			}
			arguments.erase(arguments.begin() + static_cast<std::ptrdiff_t>(i), arguments.begin() + static_cast<std::ptrdiff_t>(i + consumed));
		}

#if !(defined(VC_PROFILER_ENABLE) && VC_PROFILER_ENABLE)
		if (options.profile) {
			VC_CORE_WARN("Profiling asked, but this build has no profiler (VC_PROFILER_ENABLE)");
			options.profile = false;
		}
#endif

		s_options = options;
		return options;
	}

	const EngineOptions& getEngineOptions() {
		return s_options;
	}
}
