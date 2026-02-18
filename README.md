# Vectrix
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.0.1-green.svg)]()

An engine made to run all of my little graphics project

![Screenshot or demo GIF](basic-preview.png)

## Features
 - Vulkan powered rendering
 - OBJ Model Loading
 - Runtime shader compilation

## Prerequisites
- [CMake 3.21.7](https://cmake.org/download/)
- [Lunar Vulkan SDK >=1.4.335.0](https://vulkan.lunarg.com/sdk/home)
- [Python 3](https://www.python.org/downloads/)
- Window 10/11 or Linux (Ubuntu and PopOs were the only distros tested)

## Installation
```bash
git clone --recurse-submodules https://github.com/RX0kas/VectrixEngine.git
cd VectrixEngine
mkdir build
cmake -S . -B ./build
cmake --build ./build
cd run
..\build\bin\Your_Platform\Debug\Sandbox.exe
```

## How to use
You can see the [Sandbox](https://github.com/RX0kas/VectrixEngine/tree/master/Sandbox/src) to better understand
#### CMakeLists.txt
```cmake
# Include your source
file(GLOB_RECURSE PROJECT_SOURCES
        Project/src/*.cpp
        Project/src/*.h
)

add_executable(Project ${PROJECT_SOURCES})

# Link your project with Vectrix
target_link_libraries(Project PRIVATE Vectrix)
target_include_directories(Project PRIVATE
        $<TARGET_PROPERTY:Vectrix,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Project PRIVATE
        $<TARGET_PROPERTY:Vectrix,INTERFACE_COMPILE_DEFINITIONS>
)

if(MSVC)
    target_compile_options(Project PRIVATE /utf-8)
endif()
```
#### main.cpp
```c++
// Include the entry point
#include <Vectrix.h>
// Create a custom layer
class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer() : Layer("Example") {
	    // Create a camera
		m_camera = std::make_unique<Vectrix::PerspectiveCamera>();
		m_camera->setPosition({0.0f,0.0f,3.0f});
		m_camera->setRotation({0.0f,-M_PI,0.0f});
        // Load a model
		m_model = std::make_unique<Vectrix::Model>(Vectrix::Model::load("./mymodel.obj"));

	    // Create a layout for the main shader
		Vectrix::ShaderUniformLayout layout;
		layout.add("time",Vectrix::ShaderUniformType::Float);
	    // Create the shader and tell it how to load Obj Files
		Vectrix::ShaderManager::createShader("main", "./main.vert", "./main.frag",layout, Vectrix::getTinyObjLayout(),true);
	    // Save the shader in a variable
		main = Vectrix::ShaderManager::instance().get("main");
	}

	// Do something each frame
	void OnUpdate(Vectrix::DeltaTime ts) override {}

    // Same but for rendering stuff
	void OnRender() override {
	    // Tell the renderer to start rendering a scene
		Vectrix::Renderer::BeginScene(*m_camera);
	    // Tell to draw the model
		Vectrix::Renderer::Submit(*main.get(),*m_model);
        // Send the frame
		Vectrix::Renderer::EndScene();
	}

	void OnEvent(Vectrix::Event &event) override {
		if (event.getEventType()==Vectrix::EventType::WindowResize)
			m_camera->recalculateMatrices();
	}
private:
	Vectrix::Own<Vectrix::PerspectiveCamera> m_camera;
	Vectrix::Ref<Vectrix::Shader> main;
	Vectrix::Own<Vectrix::Model> m_model;
};

// Create a custom application
class Sandbox : public Vectrix::Application {
public:
	Sandbox() {
		m_exampleLayer = std::make_shared<ExampleLayer>();
		PushLayer(m_exampleLayer);
	}
	~Sandbox() override = default;
private:
	Vectrix::Ref<ExampleLayer> m_exampleLayer;
};
```
## Documentation

- [API Documentation (TODO)]()


## Roadmap

- [x] Clearly separate the Vulkan part from the rest of the engine
- [x] Make a universal function for sending shader uniform
- [ ] Refactor to have the same naming convention everywhere and add better comments
- [ ] Create the API documentation
- [ ] Find a better way to deal with the ImGuiLayer
- [ ] Add a material system
- [ ] Add an editor

## Known Issues

- Multi-Viewport is disabled on Linux due to numerous compatibility issues

## Acknowledgments
- Thanks to [TheCherno](https://github.com/TheCherno) for the API inspiration


## Third-party libraries
- [GLFW](https://github.com/glfw/glfw): to create the window
- [GLM](https://github.com/g-truc/glm): a mathematics library
- [Dear ImGui](https://github.com/ocornut/imgui): for the ui library
- [ShaderC](https://github.com/google/shaderc): used to compile Vulkan Shaders
- [spdlog](https://github.com/gabime/spdlog): for the logging library
- [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): Vulkan memory allocation library
- [xxHash](https://github.com/Cyan4973/xxHash): for the hashing functions
---

If you like this project, feel free to give it a star!