#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <set>

#include "volk.h"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif // !GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#ifdef VC_PLATFORM_WINDOWS
// Without NOMINMAX, Windows.h defines min/max macros that break every std::min/std::max after it
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif