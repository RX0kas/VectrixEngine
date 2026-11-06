#pragma once

/**
 * @file version.h
 * @brief The version of the engine, as macros usable at compile time
 * @ingroup core
 */

/**
 * @brief The full engine version, as a string, ending in "-WIP" while #VC_VERSION_WIP is 1
 * @ingroup core
 */
#define VC_VERSION "0.6.0-WIP"

/**
 * @brief The major part of the engine version, bumped by a breaking change
 * @ingroup core
 */
#define VC_VERSION_MAJOR 0

/**
 * @brief The minor part of the engine version, bumped by a new feature
 * @ingroup core
 */
#define VC_VERSION_MINOR 6

/**
 * @brief The patch part of the engine version, bumped by a fix
 * @ingroup core
 */
#define VC_VERSION_PATCH 0

/**
 * @brief 1 while this version is in development, 0 once it is released
 *
 * Set by Vectrix/scripts/set_version.py: a new version starts as a work in progress, `--release` clears it.
 * A build of a work in progress says so (window title, debug window, profiler), so a development build published
 * by mistake can't pass for the release.
 * @ingroup core
 */
#define VC_VERSION_WIP 1
