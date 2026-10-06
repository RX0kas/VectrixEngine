# CLAUDE.md

## Project Overview

- VectrixEngine is a C++20 game engine workspace built with a single root `CMakeLists.txt` (`CMAKE_CXX_STANDARD 20`, `CMAKE_CXX_STANDARD_REQUIRED ON` — raised from C++17 in commit `21eeb26`; concepts such as `std::derived_from` are used in public APIs, e.g. `Application::PushLayer<T>()`).
- The root project is `VectrixWorkspace` version `0.5.1`.
- Targets:
  - `Vectrix`: static engine library.
  - `VectrixEditor`: editor executable linked against `Vectrix`. Only built with `-DVECTRIX_BUILD_EDITOR=ON` (the option is `OFF` by default so apps using the engine as a library don't build it; the `CMakePresets.json` presets turn it on).
  - There is no `Sandbox` target anymore (removed with the `feature/editor` merge, PR #7).
- Current engine focus from `README.md`: Vulkan rendering, OBJ model loading, runtime shader compilation, and batch rendering.

## Repository Map

- `Vectrix/src/Vectrix/`: public engine-facing API and core systems.
  - `Core/`: application/window/logging/timing/version primitives.
  - `Rendering/`: renderer abstraction, buffers, meshes, shaders, textures, cameras, framebuffers.
  - `Scene/`: ECS wrapper around EnTT, entities, components, scene serialization.
  - `ImGui/`: engine UI layer/widget abstraction.
  - `Assets/`: `AssetsManager` (typed loading, asset ids, fingerprints for finding moved files).
  - `Project/`: `ProjectSerializer`, the binary `.vcproj` project file.
  - `Utils/`: `Json`, `Path` (UTF-8 path conversion), `Folders` (config/state/project folders), `Memory` (`Cache`), hashing, etc.
  - `Physics/`, `Events/`, `Input/`, `Layers/`, `Debug/`, `Settings/`: supporting engine systems.
  - `EntryPoint.h`: defines `main`, rebuilds a UTF-8 `argv` (`Core/CommandLine.h`) and calls the app's `createApplication(argc, argv)`. Apps name themselves with `VC_SET_APP_INFO(name, major, minor, patch)` — the name also picks the settings folder (see Settings System).
- `Vectrix/src/GraphicAPI/Vulkan/`: Vulkan backend implementation for the renderer, shaders, buffers, framebuffers, textures, swapchain, device, and ImGui integration (custom ImGui GLFW/Vulkan backends live in `GraphicAPI/Vulkan/ImGui/`; they are project code, not vendored).
- `Vectrix/src/Platform/Linux/` and `Vectrix/src/Platform/Windows/`: platform-specific input and command line (`LinCommandLine.cpp`/`WinCommandLine.cpp`). CMake globs only the current OS's folder, so every function declared for both platforms needs a file in each.
- `Vectrix/src/dox/`: Doxygen source pages (`changelog.dox` records public API changes).
- `VectrixEditor/src/`: the editor.
  - `Layers/StartupLayer`: start screen (recent projects, new/open project); switches to `Layers/EditorLayer` (scene editing, menus, viewport, save/unsaved prompts, relinking moved assets) via `Application::switchToLayer<T>`.
  - `Panels/`: `SceneHierarchyPanel` (hierarchy + properties), `ContentBrowserPanel`, `SettingsPanel`.
  - `Undo/`: command-pattern undo/redo (`UndoHistory`, owned by `EditorLayer`, cleared on scene change; `markClean`/`markModified`/`isClean` drive the unsaved `*` in the title).
  - `Utils/`: `Gizmo.h` (ImGuizmo), `Error.h` (error popups).
- `run/assets/`: the engine's own assets (`icons/`, `shaders/`), found through `AssetsManager::getEngineAssetsPath()` = `./assets`, hence running from `run/`. Game assets live in the project's `Assets/` folder.
- `demoProject/`: sample project (`demoProject.vcproj`, `Assets/{models,shaders,textures}`, `Scenes/demoProject.vctx`) used for manual testing.
- `docs/`: generated Doxygen HTML output. Prefer editing source comments or `Vectrix/src/dox`; regenerate docs only when requested.
- `Vectrix/extern/`: third-party library directory and git submodules. Do not edit vendored dependencies unless the user explicitly asks.

## Third-Party Libraries

`Vectrix/extern` contains vendored/submodule dependencies: GLFW, GLM, Dear ImGui, ImGuizmo, shaderc, glslang, SPIRV-Headers, SPIRV-Tools, VulkanMemoryAllocator, Volk, spdlog, xxHash, EnTT, NativeFileDialog, PlatformFolders (`sago::`; returns UTF-8 strings), and supporting includes.

Rules for agents:

- Treat `Vectrix/extern` as third-party code.
- Do not reformat, refactor, or patch files under `Vectrix/extern` unless explicitly requested.
- If a build problem appears in third-party code, first look for CMake options, include order, wrapper code, or integration issues in project-owned code.
- Do not add new production dependencies or submodules without asking first.

## Build System

- CMake minimum version: `3.28`.
- Language standard: C++20.
- CMake exports `compile_commands.json`.
- If `CMAKE_BUILD_TYPE` is omitted, the project defaults to `Release`.
- For single-config generators, `Dist` is normalized to `Release` early in the root CMake file.
- Output binaries/libraries are placed under the build tree in `bin/<BuildType>-<SystemName>-<Processor>`.
- Source files are collected with `file(GLOB_RECURSE ...)`; new `.cpp`/`.h` files under existing source trees are picked up by reconfigure/build, but new top-level source areas may need CMake changes.

Required tools/dependencies:

- CMake `>= 3.28`.
- Python 3 interpreter.
- Vulkan SDK / Vulkan development files. README targets Lunar Vulkan SDK `>= 1.4.335.0`.
- Vulkan 1.3 capable GPU with SPIR-V 1.6 support for runtime execution.
- Linux builds require X11 and GTK3 development packages in addition to Vulkan/GLFW-related packages.
- CI builds Ubuntu and Windows with Ninja in Debug and Release.

Common configure/build commands:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVECTRIX_BUILD_EDITOR=ON
cmake --build build --parallel
```

If Ninja is unavailable, omit `-G Ninja` and use the local default generator. Without `-DVECTRIX_BUILD_EDITOR=ON` only the engine library is built.

Local build trees (also what `cmake --preset debug|release|dist` create, editor on):

```sh
cmake --build cmake-build-debug -j6
cmake --build cmake-build-release -j6
```

Use `-j6` rather than an unbounded `--parallel` on the user's machine.

- Built-in shaders `imgui`, `mask` and `outline` (`run/assets/shaders/*.vcshader`) are compiled into the library by `vectrix_embed_shader` in the root `CMakeLists.txt` (generated headers): editing those files needs a rebuild, not just a restart.

Run graphical executables from `run/` so relative asset paths resolve:

```sh
cd run
../cmake-build-debug/bin/Debug-Linux-x86_64/VectrixEditor
```

Adjust the binary directory for the actual build type, OS, processor, and build folder. The editor also takes a `.vcproj` or `.vctx` path as its first argument (file association) and opens it directly.

## Verification Expectations

- There is currently no CMake test target, no `enable_testing()`, and no discovered unit test suite.
- For code changes, at minimum configure/build the smallest relevant build tree when dependencies are available.
- For engine or renderer changes, prefer a Debug build first. Add a Release build when the change is performance-sensitive or affects optimization-sensitive code.
- For editor, rendering, shader, asset, or windowing changes, run `VectrixEditor` only when the environment has a working display, Vulkan driver, and GPU access. If not available, state that runtime smoke testing could not be performed.
- For shader changes, ensure affected `.vcshader` files still compile at runtime when a graphical smoke test is possible.
- Running the editor rewrites the user's recent-projects list, `<StateDir>/Vectrix/recentProjects.json` (`getVectrixStateFolder()`; `~/.local/state/Vectrix/` on Linux), and any project it creates or opens is added to it. Back that file up before an automated/test run and restore it afterwards, and don't leave test projects in it. Keep the backup outside the session scratchpad, which can be wiped between turns.
- Temporary debugging/test code (readbacks, forced states, extra logs) is marked `// TMP` and removed before finishing; check with a search for `TMP` that none is left.
- For documentation-only changes, no build is required unless generated docs are explicitly requested.
- Do not claim performance improvements without measurement, profiler data, or a clearly stated rationale.

## Coding Conventions

- Match nearby style. This codebase currently mixes tabs and spaces in older files; avoid broad whitespace-only churn.
- Use namespace `Vectrix` for engine/editor code.
- Naming prefixes (the user's convention, follow it in new code):
  - `m_` for private/protected member variables of a class (`m_window`, `m_eventQueue`).
  - `s_` for static variables, static members and file-scope `static` variables (`s_instance`, `s_GLFWWindowCount`).
  - `k_` for constants (`static constexpr` members, named constants: `k_maxEntries`, `k_initialCapacity`).
  - `g_` for globals (`g_getAppInfo`).
  - No prefix for the public data members of plain structs and events (`KeyMods::ctrl`, `WindowResizeEvent::width`).
- Use the existing `VC_` macro family for platform, logging, assertions, profiling, and app metadata.
- Use `VC_CORE_*` logging for engine/internal messages and `VC_*` logging for client-facing/application messages.
- Use `VC_CORE_ASSERT` / `VC_ASSERT` where nearby code uses assertions. Assertions and logging behavior differ between Debug and Release.
- Use `std::shared_ptr` for engine objects that are shared across systems, matching existing assets/entities/layers patterns.
- Use `std::unique_ptr` for owned managers and backend resources where ownership is singular.
- Keep raw `new` usage exceptional and consistent with existing factory/entry-point conventions.
- Add `VC_PROFILER_FUNCTION()` in performance-sensitive engine/backend functions when the surrounding file already uses profiler instrumentation.
- Preserve public API names and conventions. Many public layer/application methods use PascalCase, while some getters use lower/camel case.
- When adding public engine API intended for users, update `Vectrix/src/Vectrix.h` if it should be included by `<Vectrix.h>`.
- Keep comments useful and concise. Prefer Doxygen comments for public engine APIs.
- **Template member functions on public engine classes (e.g. `Application`) must be defined inline in the header, never in the `.cpp`.** `Vectrix` is a static library; `VectrixEditor` and user apps are separate link targets that instantiate these templates with their own types (e.g. `Application::instance().PushLayer<EditorLayer>()` from `VectrixEditor`). A definition that only lives in `Application.cpp` compiles fine but fails at **link time** with an undefined-reference error in the consuming target, because that instantiation was never emitted into `libVectrix.a`. This bit `PushLayer<T>`/`PushOverlay<T>`/`PopLayer<T>`/`PopOverlay<T>`/`switchToLayer<T>` in `Application.h`/`.cpp` in this exact way — if you see a link error for a templated `Application`/engine method used from the editor or an app, check this first before assuming it's something else.

## Architecture Rules

- Keep public engine abstractions in `Vectrix/src/Vectrix` independent from Vulkan-specific implementation details where practical.
- Put Vulkan-specific work under `Vectrix/src/GraphicAPI/Vulkan`.
- Put OS-specific work under `Vectrix/src/Platform/Linux` or `Vectrix/src/Platform/Windows`.
- `RendererAPI::API` currently exposes `None` and `Vulkan`; do not introduce another backend without a plan for the abstraction boundary.
- Respect the layering:
  - Application/layers drive update, render, ImGui, and event callbacks.
  - `Renderer` is the public static rendering facade.
  - `RenderCommand` / `RendererAPI` bridge public rendering to backend implementation.
  - Vulkan renderer/device/swapchain/buffer/shader classes own backend details.
  - Scene/entity/component code wraps EnTT and should not depend on Vulkan.
- Be careful with frame ownership, Vulkan object lifetimes, descriptor layouts, swapchain recreation, and frames-in-flight behavior.
- Avoid hot-path allocations in rendering, scene iteration, shader binding, asset lookup, and per-frame UI code unless justified.
- Prefer measured, localized optimizations over broad rewrites.

## Settings System

- `Vectrix/src/Vectrix/Settings/SettingsManager.h` owns a **two-tier** JSON settings store: a `Global` tier loaded once at start-up from `<ConfigHome>/Vectrix/<AppName>/settings.vectrix.json` (`getConfigFolder()`, `Vectrix/src/Vectrix/Utils/Folders.h`; `<ConfigHome>` is `sago::getConfigHome()` from PlatformFolders — e.g. `~/.config` on Linux) (`Application::Application()`, before the window/Vulkan context exist — some fields, like `window.*` and `engine.logging.level`, are read directly at that point), and a `Project` tier loaded from `<projectDir>/settings.vectrix.json` when a project/scene is opened (`EditorLayer::openScene`, from the project root, not the scene's `Scenes/` folder — an older bug wrote it there, which is where the leftover `demoProject/Scenes/settings.vectrix.json` comes from). `SettingsManager::getSettings()` returns the two merged (project overrides global, key by key, recursively) — read from this everywhere; never read `m_global`/`m_project` directly. `SettingsManager::tier(Scope)` gives the mutable object for one tier (used by the settings UI); call `markChanged()` after mutating it and `save(Scope)` to persist it.
- `settings.vectrix.json` schema (top-level keys): `engine.graphicAPI.vulkan.{startingDescriptorPoolConfig,swapchain,device,textures,shaders,rendering}`, `engine.rendering.clearColor` (`[r,g,b,a]`), `engine.logging.level`; `window.{resizable,width,height,title}`; `editor.{camera,gizmo,contentBrowser,ui.{docking,viewports,dpiScaling,theme},outline}`. `VulkanSettings::load()` (`Vectrix/src/GraphicAPI/Vulkan/VulkanSettings.h`) reads the `engine.graphicAPI.vulkan.*` subtree into a typed struct consumed by `VulkanContext`/`Device`/`SwapChain`/textures/shaders/pipeline.
- Most settings only take effect on the next launch (they're read once at start-up). The exceptions wired to apply live are: `editor.rendering`/clear color, `editor.ui.theme`, `editor.camera.moveSpeed`/`rotationSpeed`, `editor.gizmo.*`, and `editor.outline.*` — re-read every frame/on save via `EditorLayer::applyLiveSettings()` and `SettingsPanel`.
- `JsonValue::getAs<T>()` (`Vectrix/src/Vectrix/Utils/Json.h`) returns `std::optional<T>`; for numeric `T` it reads the underlying `double` via `if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)` — do not try `std::get<T>`/`std::holds_alternative<T>` directly for integer types (`uint32_t` etc. aren't a variant alternative, so that fails to compile with a `static_assert`). Both `operator[]` overloads and the scalar getters are intentionally silent (no `VC_CORE_ERROR`) on a missing/wrong-typed key so that reading from a not-yet-populated settings tree at start-up doesn't abort a Debug build — keep that pattern if you touch `Json.h`.
- `editor.ui.viewports` (ImGui multi-viewport / floating platform windows) is force-disabled on Wayland regardless of the setting, in `VulkanImGuiManager::initImGui()`, via `Window::getDisplayServer() != WAYLAND` — GLFW+Vulkan multi-viewport surface creation is the one other call site of `glfwCreateWindowSurface` in this codebase besides `Device::createSurface()`, so a null-window GLFW assertion at start-up on Linux is worth checking against this flag/session type first.

## Known Gotchas

- **`Cache<K,I>`** (`Vectrix/src/Vectrix/Utils/Memory.h`) is a thin `public`-derived wrapper around `std::unordered_map<K,I,XXH3>`. Use its own public `iterator`/`begin()`/`end()` (or add a `using iterator = Cache<K,I>::iterator;` alias where needed) — do **not** reach for libstdc++-internal typedefs like `_Hashtable` (a private member of `std::unordered_map`, so `Cache<...>::_Hashtable::iterator` fails to compile/is not portable across standard library implementations). Also remember `unordered_map` iterators are forward-only (no `operator--`) and its order is unspecified, so don't use a `Cache` where order matters (`LayerStack` briefly did; it now keeps its layers in `std::vector`s, in push order).
- **Concurrent editing from an IDE**: this repo is commonly open in CLion (its bundled `cmake` binary shows up in build command lines, e.g. under `~/.local/share/JetBrains/Toolbox/...`) at the same time as Claude Code sessions. If a file you just edited appears to have reverted or picked up unrelated changes shortly after, or a `cmake --build` fails linking with `ar: <file>.o: No such file or directory` right after a clean compile, that's very likely CLion's own build and/or file-watcher/autosave racing with yours in the same `cmake-build-*` directory and source tree — not a logic bug. Re-check the file's current on-disk content before re-editing, and consider deleting the specific stale `.o` under `cmake-build-*/CMakeFiles/.../*.o` to force a rebuild if ninja treats a file as up to date (by mtime) despite it visibly lacking a symbol the current source defines.
- **`VC_CORE_ASSERT`/`VC_CORE_ERROR`/`VC_CORE_CRITICAL`/`VC_ASSERT` compile to nothing in Release builds** (`Vectrix/src/Vectrix/Core/Core.h`, `Log.h`) — they only abort in Debug. Don't rely on them for Release-mode control flow or error propagation, and never let a non-`void` function fall off its end after a bare `VC_CORE_ERROR`/`VC_CORE_CRITICAL` call with no following `return` — that compiles (the macro can be empty) but is undefined behavior in Release. This bit `Window::detectLinuxDisplayServer()` and `Json::parseBool()` (both fixed: they now log via `VC_CORE_ERROR_NO_EXIT` and explicitly `return` a fallback value) — if you add a new terminal error branch in a non-`void` function, give it an explicit `return` regardless of which logging macro you use. Also watch for the same failure mode as a `switch`-case fallthrough: a `case RendererAPI::API::None:` branch that only logs and doesn't `return`/`break` will, in Release, fall into the next case's real work — every factory `create()` in `Rendering/` was patched to `return nullptr;` after that case for this reason.
- **Paths held in a `std::string` are UTF-8** (file dialogs, JSON, scene/project files, asset ids, ImGui text, logs, and the `argv` given to `createApplication`, which `EntryPoint.h` rebuilds from the wide command line on Windows via `getCommandLineArguments`). Convert with `toUtf8`/`toGenericUtf8`/`fromUtf8` from `Vectrix/src/Vectrix/Utils/Path.h`; never call `path::string()`/`generic_string()` or build a `std::filesystem::path` straight from a `std::string` (including implicitly, e.g. `folder / someStdString`). On Windows those use the system code page instead of UTF-8, which garbles accented names and throws on characters the code page can't represent; on Linux both behave the same, so the bug only shows on Windows. Use `toGenericUtf8` for anything written to a file read on other machines (asset ids in scenes, the project's start scene), and open files through a path (`std::ifstream(fromUtf8(pathString))`), not a narrow string.
- **GPU resources may still be in use by the previous frame.** `SwapChain::MAX_FRAMES_IN_FLIGHT` is 2. Don't destroy a texture, framebuffer, buffer or descriptor the GPU may still read, and don't call `vkDeviceWaitIdle` for it: pass the destruction to `VulkanContext::destroyWhenUnused(fn)`, which runs it once those frames are done (or right away, after a device wait, when no renderer exists). Per-frame data (shader SSBOs, object data, descriptor sets) is kept once per frame in flight and indexed with `getFrameIndex()`.
- **Events are subscriptions, queued per frame** (`Vectrix/src/Vectrix/Events/`). `Layer` and `Application` derive from `EventListener`: subscribe once (constructor or `OnAttach`) with `subscribe<T>(handler)` — the handler takes `const T&` or nothing and returns `bool` (true = consumed) or `void`; `subscribe<Event>` gets everything. There is no `OnEvent` anymore. The window's GLFW callbacks only *post* to `Application`'s `EventQueue`; `Application::run()` sends the batch at the start of the next frame, before `OnUpdate`: ImGui (`ImGuiLayer::capturesEvent`), overlays then layers top-down, then the Application's own handlers; an unconsumed `WindowCloseEvent` stops the app. Custom events derive from `EventBase<Self, "Name", categories>` and are sent with `Application::instance().postEvent<T>(args...)` (any thread; other threads' events go through a locked inbox and come after the main thread's) or `sendEvent<T>(args...)` (main thread, immediate, returns whether it was consumed). An object that isn't a listener but subscribes to one (a panel to its layer) keeps the `ScopedSubscription` from `subscribeScoped<T>` as a member, so the handler can't outlive it. `Application::observeEvents` sees every event after dispatch, consumed ones included, with who consumed them (the editor's Event Log uses it). The editor's own events (`AssetMovedEvent`, `SceneOpenedEvent`, `SelectionChangedEvent`) are in `VectrixEditor/src/Events/EditorEvents.h`; the content browser reports moves by `sendEvent<AssetMovedEvent>` (synchronous, `EditorLayer::onAssetMoved` must run before the next draw) and imports `FilesDroppedEvent` paths into its current folder. Window callbacks are installed in `Window::init` before ImGui's GLFW backend, which chains to them — keep that order when adding a callback.
- **The Vulkan Debug window is opt-in.** An app shows it by calling `imguiLayer().getManager().renderDebugGraphicWidget(bool& open)` from its ImGui code (the editor does from Window > Graphics Debug); it is not attached automatically anymore, unlike on older `master`.
- `rg` is not always on the shell `PATH` of these sessions; fall back to `grep -rn` (excluding `Vectrix/extern`) when it is missing.

## Shaders

- `.vcshader` files contain multiple sections marked with `#shader frag` and `#shader vert`. Engine shaders live in `run/assets/shaders/`; a project's shaders are assets in its `Assets/` folder (e.g. `demoProject/Assets/shaders/viewport.vcshader`).
- Shader code is GLSL 450 and is compiled at runtime through shaderc for Vulkan 1.3 / SPIR-V 1.6.
- Descriptor layout every shader follows (`ShaderSSBO`, `DynamicSSBO`, `VulkanShader::createPipelineLayout`):
  - set 0, binding 0: `readonly buffer FrameSSBO { ... } frame;` — the shader's uniforms. Its member list is parsed from the GLSL source (`Shader::findShaderUniformLayoutFromSource`) with std430 rules, so the C++ offsets come from that text: `vec3` takes 16 bytes, and the block must be declared identically in both stages. Set values with `setUniform*`; a name or type mismatch logs and does nothing.
  - set 0, binding 1: `uniform sampler2D u_Textures[128];` (`Texture::getMaxTexturePerShader()`), indexed with `nonuniformEXT` (needs `#extension GL_EXT_nonuniform_qualifier : enable`). `Shader::useTexture`/`useFramebuffer` return the slot to read; slot 0 is always the not_found texture (what you get when all slots are taken), and slots of destroyed textures are reused. Never hard-code a slot index in GLSL — pass it in, as the outline does with `u_MaskIndex`.
  - set 1, binding 0: `readonly buffer ObjectSSBO` — per-object data for batched draws (`DynamicSSBO`), read with the instance/object index.
- `vc_cameraTransform` is a reserved uniform name: a `FrameSSBO` containing it marks the shader as camera-affected and the renderer fills it. Do not use it for custom uniforms.
- Built-in shaders are embedded at build time (see Build System) and registered under names like `builtin:mask`/`builtin:outline` (`VulkanShader::k_*ShaderName`), which no asset id can collide with.
- All descriptor sets come from `Device::descriptorPool()` (sizes from `engine.graphicAPI.vulkan.startingDescriptorPoolConfig`; it does not grow), except ImGui's own pool in `VulkanImGuiManager`. Textures and framebuffers also allocate one ImGui descriptor set each from it, for `ImGui::Image`.

## Assets, Projects And Scenes

- Load assets through `AssetsManager::load<T>(path)` (or `VC_LOAD_ASSET`). A relative path is taken from the current project's `Assets/` folder (`setAssetsPath`); without a project it falls back to the engine assets folder (`getEngineAssetsPath()`, `./assets`). Engine/editor assets (e.g. icons) go through `getEngineAssetsPath()` explicitly.
- An asset's **id** is its path relative to the assets folder with `/` separators (`toGenericUtf8`), or the absolute path for a file outside it. Ids are what scenes store and what managers cache under; a file always gets the same id however it was requested.
- A project is a folder with a binary `<name>.vcproj` (`ProjectSerializer`: magic, engine/file version, name, start scene relative to the project), `Assets/`, `Scenes/` and an optional `settings.vectrix.json` at its root. Default location: `~/Documents/VectrixProject` (`DefaultVectrixProjectPath`).
- Scenes are binary `.vctx` files (`SceneSerializer`). Next to each asset id they store an `AssetFingerprint` (size + XXH3 of the content).
- Missing assets: a scene still opens; each component keeps the path of its missing asset (`MeshRendererComponent::missingMesh`, ...) and saving writes it back, so nothing is lost, and the user is told which are missing. When the content browser moves/renames a file, `EditorLayer::onAssetMoved` updates loaded assets, the open scene and every scene file of the project. For files moved outside the editor, `EditorLayer::relinkMovedAssets` uses `AssetsManager::findMovedAsset`: a unique content match is relinked in every scene of the project; a unique same-name match with different content is only a guess (`byNameOnly`), applied to the open scene alone and marked unsaved (`UndoHistory::markModified`); an ambiguous match is left missing.
- When changing asset, scene or project formats, bump the matching version and mention migration impact for existing projects.

## Documentation

- Doxygen is configured by `Doxyfile`.
- Doxygen input is `Vectrix/src/Vectrix` and `Vectrix/src/dox`.
- Generated output goes to `docs/`.
- Update source Doxygen comments or `.dox` pages for public API changes.
- Only regenerate `docs/` when requested or when the task explicitly includes generated documentation.

## Git And Workspace Hygiene

- The worktree may contain user changes. Inspect files before editing and never revert unrelated changes.
- Keep changes scoped to the user request.
- Do not edit generated build directories such as `build/`, `cmake-build-*`, or generated binary output.
- Do not delete or rewrite assets, docs, or sample code unless the user asks.
- Prefer `rg` / `rg --files` for repository search.
- Before finalizing code changes, summarize what changed and exactly what was verified.

## Review Checklist For Agents

When reviewing or implementing changes, check for:

- Build breakage on Linux/Windows assumptions.
- Public API changes that need umbrella include or docs updates.
- Lifetime/ownership errors with `shared_ptr`, `unique_ptr`, Vulkan handles, and frame resources.
- Renderer/backend abstraction leaks.
- Per-frame allocations or unnecessary synchronization.
- Shader layout mismatches between C++ buffers/descriptors and GLSL.
- Asset path assumptions that only work from one working directory.
- Missing verification, especially when graphical smoke tests cannot run.

<!-- code-review-graph MCP tools -->
## MCP Tools: code-review-graph

**This project has a knowledge graph. Start with the code-review-graph
MCP tools to narrow scope, then read the source.** The graph is cheaper than scanning files and
gives you structural context (callers, dependents, test coverage) that file search cannot.

### When to use graph tools FIRST

- **Exploring code**: `semantic_search_nodes_tool` or `query_graph_tool` instead of Grep
- **Understanding impact**: `get_impact_radius_tool` instead of manually tracing imports
- **Code review**: `detect_changes_tool` + `get_review_context_tool` instead of reading entire files
- **Finding relationships**: `query_graph_tool` with callers_of/callees_of/imports_of/tests_for
- **Architecture questions**: `get_architecture_overview_tool` + `list_communities_tool`

### Verify in the source

- Narrow scope with the graph, then read the source. Do not change code from graph output alone.
- For any non-trivial change, read the implementation and the relevant tests before concluding.
- Verify the exact source when touching behavior, database logic, migrations, retries, fallbacks,
  recovery, or compatibility code.
- When the graph and the source disagree, the source wins. The graph may be stale or may not
  model that relationship.
- An empty graph result can mean "not indexed" or "not statically visible", not "does not exist".

### Key Tools

| Tool | Use when |
| ------ | ---------- |
| `detect_changes_tool` | Reviewing code changes — gives risk-scored analysis |
| `get_review_context_tool` | Need source snippets for review — token-efficient |
| `get_impact_radius_tool` | Understanding blast radius of a change |
| `get_affected_flows_tool` | Finding which execution paths are impacted |
| `query_graph_tool` | Tracing callers, callees, imports, tests, dependencies |
| `semantic_search_nodes_tool` | Finding functions/classes by name or keyword |
| `get_architecture_overview_tool` | Understanding high-level codebase structure |
| `refactor_tool` | Planning renames, finding dead code |

### Workflow

1. The graph auto-updates on file changes (via hooks).
2. Use `detect_changes_tool` for code review.
3. Use `get_affected_flows_tool` to understand impact.
4. Use `query_graph_tool` pattern="tests_for" to check coverage.
<!-- /code-review-graph MCP tools -->
