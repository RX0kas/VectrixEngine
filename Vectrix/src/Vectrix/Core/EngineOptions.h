#ifndef VECTRIXWORKSPACE_ENGINEOPTIONS_H
#define VECTRIXWORKSPACE_ENGINEOPTIONS_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

/**
 * @file EngineOptions.h
 * @brief The command line options the engine itself understands (`--vc-...`)
 * @ingroup core
 */

namespace Vectrix {
	/**
	 * @brief What the engine was asked on the command line (and through the environment)
	 *
	 * The options start with `--vc-`. EntryPoint.h reads them and removes them from the arguments before
	 * createApplication, so an application never sees them (e.g. the editor doesn't take `--vc-frames` for a
	 * file to open):
	 * - `--vc-profile`, or the environment variable `VC_PROFILE=1`: record the profiler sessions (startup,
	 *   runtime, shutdown). Off by default. The files go to the application's state folder,
	 *   `<StateDir>/Vectrix/<AppName>/profiles/` (`~/.local/state/...` on Linux), or to the folder given with
	 *   `--vc-profile=<folder>` / `VC_PROFILE=<folder>`. Only in a build with #VC_PROFILER_ENABLE (Debug).
	 * - `--vc-frames=<N>` (or `--vc-frames <N>`): stop after N frames, as if Application::close had been
	 *   called, and return 0. For a start-up test that tells "it runs" from "it's stuck" (e.g. under ctest).
	 * @see getEngineOptions
	 * @ingroup core
	 */
	struct EngineOptions {
		/// Whether the profiler sessions are recorded
		bool profile = false;
		/// Where the profile files go, empty for the default folder (see profileOutputFolder)
		std::filesystem::path profileFolder;
		/// How many frames to run before stopping, none to run until closed
		std::optional<std::uint64_t> frameLimit;

		/**
		 * @brief The folder the profile files are written to
		 * @return profileFolder, or `<StateDir>/Vectrix/<AppName>/profiles` when it is empty
		 */
		[[nodiscard]] std::filesystem::path profileOutputFolder() const;
	};

	/**
	 * @brief Read the engine's options out of the arguments, and remove them
	 *
	 * Also reads the environment (`VC_PROFILE`). An unknown `--vc-` option or a bad value is logged and
	 * removed too. Called by EntryPoint.h, before createApplication; the result is kept for getEngineOptions.
	 * @param arguments The command line in UTF-8, the program's name first: the engine's options are erased
	 * @return The options found
	 * @ingroup core
	 */
	EngineOptions extractEngineOptions(std::vector<std::string>& arguments);

	/**
	 * @brief The options extractEngineOptions found, the defaults before it ran
	 * @ingroup core
	 */
	const EngineOptions& getEngineOptions();
}

#endif //VECTRIXWORKSPACE_ENGINEOPTIONS_H
