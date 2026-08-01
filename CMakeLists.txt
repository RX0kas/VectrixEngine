cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Dist")
  set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


project(VectrixWorkspace VERSION 0.5.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OUTPUT_DIR ${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./bin/${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin/${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin/${OUTPUT_DIR})


message(STATUS "Initializing Vectrix dependencies")

# Python
message(VERBOSE "Searching python interpreter")
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Vulkan
message(VERBOSE "Initializing Vulkan")

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED)
# Volk
message(VERBOSE "Initializing Volk")
set(VOLK_STATIC_DEFINES)
add_subdirectory(${PROJECT_SOURCE_DIR}/Vectrix/extern/Volk)

# Compiling Shader
# Setup SPIRV-Tools
message(VERBOSE "Initializing SPIRV-Header")
set(SPIRV-Headers_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Vectrix/extern/SPIRV-Headers)
set(SHADERC_SPIRV_HEADERS_DIR SPIRV-Headers_SOURCE_DIR)
message(VERBOSE "Initializing SPIRV-Tools")
set(tools_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Vectrix/extern/SPIRV-Tools)
set(glslang_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Vectrix/extern/glslang)
add_subdirectory(Vectrix/extern/SPIRV-Tools)

message(VERBOSE "Initializing glslang")
add_subdirectory(Vectrix/extern/glslang)

add_subdirectory(Vectrix/extern/shaderc)
# VulkanMemoryAllocator
message(VERBOSE "Initializing VulkanMemoryAllocator")
add_subdirectory(Vectrix/extern/VulkanMemoryAllocator)

# GLFW
message(VERBOSE "Initializing GLFW")
add_subdirectory(Vectrix/extern/GLFW)

# spdlog
message(VERBOSE "Initializing spdlog")
add_subdirectory(Vectrix/extern/spdlog)

# xxHash
message(VERBOSE "Initializing xxHash")
set(XXHASH_BUILD_XXHSUM OFF)
add_subdirectory(Vectrix/extern/xxHash/cmake_unofficial EXCLUDE_FROM_ALL)

# EnTT
message(VERBOSE "Initializing EnTT")
add_subdirectory(Vectrix/extern/EnTT)

# ImGui
message(VERBOSE "Initializing ImGui")
file(GLOB IMGUI_SOURCES
        Vectrix/extern/imgui/*.cpp
)

add_library(imgui STATIC
        ${IMGUI_SOURCES}
)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
target_link_libraries(imgui volk::volk glfw)

target_include_directories(imgui PUBLIC Vectrix/extern/imgui)

# ImGuizmo
message(VERBOSE "Initializing ImGuizmo")
add_library(imguizmo STATIC
        Vectrix/extern/ImGuizmo/src/ImGuizmo.cpp
        Vectrix/extern/ImGuizmo/src/ImGuizmo.h
)

target_link_libraries(imguizmo PUBLIC imgui)
target_include_directories(imguizmo PUBLIC Vectrix/extern/imgui)

# nativefiledialog-extended
message(VERBOSE "Initializing nativefiledialog-extended")
if (UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK3 gtk+-3.0)

  if (GTK3_FOUND)
    message(VERBOSE "GTK3 found")
  else()
    message(FATAL_ERROR "GTK3 not found, install libgtk-3-dev")
  endif()
endif()
add_subdirectory(Vectrix/extern/NativeFileDialog)

# Vectrix
message(STATUS "Initializing Vectrix")

file(GLOB_RECURSE Vectrix_SOURCES
        Vectrix/src/Vectrix/*.cpp
        Vectrix/src/Vectrix/*.h
)

list(APPEND Vectrix_SOURCES
        Vectrix/src/vcpch.cpp
        Vectrix/src/vcpch.h
        Vectrix/src/Vectrix.h
        Vectrix/extern/Volk/volk.c
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
        Vectrix/extern/ImGuizmo/src
        ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions(Vectrix PRIVATE
        VC_STATIC_LIB
        IMGUI_IMPL_VULKAN_USE_VOLK
)

if(WIN32)
  target_compile_definitions(Vectrix PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_sources(Vectrix PRIVATE ${IMGUI_SOURCES})

if (UNIX AND NOT APPLE)
  # For Linux we need X11
  find_package(X11 REQUIRED)
  target_link_libraries(Vectrix
            glfw
            shaderc
            X11::X11
            xxHash::xxhash
            GPUOpen::VulkanMemoryAllocator
            volk::volk
            EnTT::EnTT
            imguizmo
            nfd
    )
else()
  target_link_libraries(Vectrix
        glfw
        shaderc
        xxHash::xxhash
        GPUOpen::VulkanMemoryAllocator
        volk::volk
        EnTT::EnTT
        imguizmo
        nfd
)
endif()

# UTF-8 (MSVC)
if(MSVC)
  target_compile_options(Vectrix PRIVATE /utf-8)
endif()

target_compile_definitions(Vectrix PUBLIC
        $<$<CONFIG:Debug>:VC_DEBUG>
        $<$<CONFIG:Release>:VC_RELEASE>
        $<$<CONFIG:Dist>:VC_DIST>
)

if(WIN32)
  target_compile_definitions(Vectrix PUBLIC
            VC_PLATFORM_WINDOWS
            GLFW_INCLUDE_NONE
    )
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(Vectrix PUBLIC VC_PLATFORM_LINUX)
endif()

##########
# Editor #
##########

file(GLOB_RECURSE EDITOR_SOURCES
        VectrixEditor/src/*.cpp
        VectrixEditor/src/*.h
)

add_executable(VectrixEditor ${EDITOR_SOURCES})

target_link_libraries(VectrixEditor PRIVATE Vectrix)

target_include_directories(VectrixEditor PRIVATE
        $<TARGET_PROPERTY:Vectrix,INTERFACE_INCLUDE_DIRECTORIES>
        VectrixEditor/src/
)

target_compile_definitions(VectrixEditor PRIVATE
        $<TARGET_PROPERTY:Vectrix,INTERFACE_COMPILE_DEFINITIONS>
)

if(MSVC)
  target_compile_options(VectrixEditor PRIVATE /utf-8)
endif()


###########
# Sandbox #
###########
file(GLOB_RECURSE SANDBOX_SOURCES
        Sandbox/src/*.cpp
        Sandbox/src/*.h
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox PRIVATE Vectrix)

target_include_directories(Sandbox PRIVATE
        $<TARGET_PROPERTY:Vectrix,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(Sandbox PRIVATE
        $<TARGET_PROPERTY:Vectrix,INTERFACE_COMPILE_DEFINITIONS>
)

if(MSVC)
  target_compile_options(Sandbox PRIVATE /utf-8)
endif()
