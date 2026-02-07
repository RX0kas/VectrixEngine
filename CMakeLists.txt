cmake_minimum_required(VERSION 4.0)
project(VectrixWorkspace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Dist")
    set(CMAKE_BUILD_TYPE Release)
endif()

set(OUTPUT_DIR ${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./bin/${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin/${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin/${OUTPUT_DIR})

# Python
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Vulkan
find_package(Vulkan REQUIRED)
# Compiling Shader
# Setup SPIRV-Tools
message(STATUS "Initializing SPIRV-Header")
set(SPIRV-Headers_SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/Vectrix/extern/SPIRV-Header)
set(SHADERC_SPIRV_HEADERS_DIR SPIRV-Headers_SOURCE_DIR)
# add_subdirectory(${CMAKE_HOME_DIRECTORY}/Vectrix/extern/SPIRV-Header)
message(STATUS "Initializing SPIRV-Tools")
add_subdirectory(Vectrix/extern/SPIRV-Tools)

message(STATUS "Initializing glslang")
add_subdirectory(Vectrix/extern/glslang)

set(SPIRV_SKIP_TESTS ON)
set(SPIRV_TOOLS_BUILD_STATIC ON)
set(ENABLE_GLSLANG_BINARIES OFF)
add_subdirectory(Vectrix/extern/shaderc)

# GLFW
add_subdirectory(Vectrix/extern/GLFW)

# spdlog
add_subdirectory(Vectrix/extern/spdlog)

# xxHash
set(XXHASH_BUILD_XXHSUM OFF)
add_subdirectory(Vectrix/extern/xxHash/cmake_unofficial EXCLUDE_FROM_ALL)

# Vectrix
file(GLOB_RECURSE Vectrix_SOURCES
        Vectrix/src/Vectrix/*.cpp
        Vectrix/src/Vectrix/*.h
)

list(APPEND Vectrix_SOURCES
        Vectrix/src/vcpch.cpp
        Vectrix/src/vcpch.h
        Vectrix/src/Vectrix.h
        Vectrix/src/tinyobj_loader.cpp
)


if (WIN32)
    message(STATUS "Windows Platform detected")

    file(GLOB_RECURSE PLATFORM_SOURCES
            Vectrix/src/Platform/Windows/*.cpp
            Vectrix/src/Platform/Windows/*.h
    )

elseif (UNIX AND NOT APPLE)
    message(STATUS "Linux Platform detected")

    file(GLOB_RECURSE PLATFORM_SOURCES
            Vectrix/src/Platform/Linux/*.cpp
            Vectrix/src/Platform/Linux/*.h
    )

else()
    message(FATAL_ERROR "Unsupported platform")
endif()

file(GLOB_RECURSE GRAPHICS_API_SOURCES
    Vectrix/src/GraphicAPI/*.cpp
    Vectrix/src/GraphicAPI/*.h
)

list(APPEND Vectrix_SOURCES ${PLATFORM_SOURCES})
list(APPEND Vectrix_SOURCES ${GRAPHICS_API_SOURCES})

add_library(Vectrix STATIC ${Vectrix_SOURCES})

if(MSVC)
    target_precompile_headers(Vectrix PRIVATE
            Vectrix/src/vcpch.h
    )
endif()


target_include_directories(Vectrix PUBLIC
        Vectrix/src
        Vectrix/extern/spdlog/include
        Vectrix/extern/glm
        Vectrix/extern/imgui
        Vectrix/extern/include
        ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions(Vectrix PRIVATE
        VC_STATIC_LIB
        IMGUI_API=
        IMGUI_IMPL_API=
)

if(WIN32)
    target_compile_definitions(Vectrix PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

file(GLOB IMGUI_SOURCES
        Vectrix/extern/imgui/*.cpp
        Vectrix/extern/imgui/backends/imgui_impl_glfw.cpp
        Vectrix/extern/imgui/backends/imgui_impl_vulkan.cpp
)

target_sources(Vectrix PRIVATE ${IMGUI_SOURCES})

if (UNIX AND NOT APPLE)
    # For Linux we need X11
    find_package(X11 REQUIRED)
    target_link_libraries(Vectrix
            glfw
            Vulkan::Vulkan
            shaderc
            X11::X11
            xxHash::xxhash
    )
else()
target_link_libraries(Vectrix
        glfw
        Vulkan::Vulkan
        shaderc
        xxHash::xxhash
)
endif()

# UTF-8 (MSVC)
if(MSVC)
    target_compile_options(Vectrix PRIVATE /utf-8)
endif()

target_compile_definitions(Vectrix PRIVATE
        $<$<CONFIG:Debug>:VC_DEBUG>
        $<$<CONFIG:Release>:VC_RELEASE>
        $<$<CONFIG:Dist>:VC_DIST>
)


# Sandbox
file(GLOB_RECURSE SANDBOX_SOURCES
        Sandbox/src/*.cpp
        Sandbox/src/*.h
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_include_directories(Sandbox PRIVATE
        Vectrix/src
        Vectrix/extern
        Vectrix/extern/spdlog/include
        Vectrix/extern/glm
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Sandbox
        Vectrix
        glfw
        Vulkan::Vulkan
        spdlog
        xxHash::xxhash
)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /utf-8)
endif()

if(WIN32)
    target_compile_definitions(Vectrix PRIVATE
            VC_PLATFORM_WINDOWS
            GLFW_INCLUDE_NONE
    )
    target_compile_definitions(Sandbox PRIVATE VC_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(Vectrix PRIVATE
            VC_PLATFORM_LINUX
    )
    target_compile_definitions(Sandbox PRIVATE VC_PLATFORM_LINUX)
endif()

target_compile_definitions(Sandbox PRIVATE
        $<$<CONFIG:Debug>:VC_DEBUG>
        $<$<CONFIG:Release>:VC_RELEASE>
        $<$<CONFIG:Dist>:VC_DIST>
)