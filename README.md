# Vectrix
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.0.1-green.svg)]()

An engine made to run all of my little graphics project

![Screenshot or demo GIF](p.png)

## Features


## Prerequisites
- [CMake 3.20](https://cmake.org/download/)
- [Lunar Vulkan SDK >=1.4.335.0](https://vulkan.lunarg.com/sdk/home)
- Window 10/11 or Linux (Ubuntu and PopOs were the only distros tested)

## Installation
```bash
# Clone the repository
git clone --recurse-submodules https://github.com/RX0kas/VectrixEngine.git
cd VectrixEngine
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -S . -B ./build
$ cmake --build ./build
```
## Documentation

- [API Documentation (TODO)]()


## Roadmap

- [ ] Finish the README
- [x] Clearly separate the Vulkan part from the rest of the engine
- [ ] Make a universal function for sending shader uniform
- [ ] Performance optimization
- [ ] Add a material system
- [ ] Add an editor

## Known Issues

- Multi-Viewport is disabled on Linux due to numerous compatibility issues

## Acknowledgments

- Thanks to [TheCherno](https://github.com/TheCherno) for the inspiration
- [Dear ImGui](https://github.com/ocornut/imgui) for the ui library
- [spdlog](https://github.com/gabime/spdlog) for the logging library

---

If you like this project, feel free to give it a star!