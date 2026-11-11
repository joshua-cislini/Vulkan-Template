# Vulkan C++23 Project Template

A professional, cross-platform, zero-friction Vulkan C++ starter template designed for building modern Vulkan applications, graphics engines, demos, and games. 

It ships with a clean, working Vulkan renderer ("Hello Triangle") as a proven, functional baseline, accompanied by automated GLSL-to-SPIR-V shader compilation, Pragmatic C++23 RAII lifecycle management, validation layers, dynamic state handling, and turnkey tooling.

Compiles out of the box on **Windows** (MSYS2 UCRT64, native `cmd.exe`, PowerShell, Git Bash, or LunarG Vulkan SDK), **Linux**, and **macOS** (MoltenVK) using **GNU Make**, **GCC (g++)** or **Clang (clang++)**, **glslc**, **clangd**, **GDB/LLDB**, and **VS Code**.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Starter Guide: Making It Your Project](#starter-guide-making-it-your-project)
   - [Renaming Your Application](#1-renaming-your-application)
   - [Adding Your Own Source Code](#2-adding-your-own-source-code)
   - [Adding & Customizing Shaders](#3-adding--customizing-shaders)
   - [Adding Third-Party Libraries](#4-adding-third-party-libraries)
   - [Adding Unit & Integration Tests](#5-adding-unit--integration-tests)
   - [Building for Production / Release](#6-building-for-production--release)
3. [Key Features](#key-features)
4. [Project Structure](#project-structure)
5. [Prerequisites & Toolchain Setup](#prerequisites--toolchain-setup)
6. [Configuration Guide (`config.mk`)](#configuration-guide-configmk)
7. [Architecture Overview](#architecture-overview)
8. [Automatic Shader Pipeline (`shaders/`)](#automatic-shader-pipeline-shaders)
9. [Drop-In Dependency Architecture](#drop-in-dependency-architecture)
10. [Unit & Headless Testing System](#unit--headless-testing-system)
11. [Code Quality & Tooling](#code-quality--tooling)
12. [Visual Studio Code Integration](#visual-studio-code-integration)
13. [Continuous Integration (GitHub Actions)](#continuous-integration-github-actions)
14. [Make Targets Reference](#make-targets-reference)
15. [Troubleshooting & FAQ](#troubleshooting--faq)
16. [License](#license)

---

## Quick Start

Clone or download this template, then open a terminal in the project directory:

```bash
# 1. Build in debug mode (compiles C++23 sources and GLSL shaders)
make

# 2. Run the application
make run

# 3. Run the automated test suites
make test

# 4. Build in release mode (optimized -O2, stripped symbols, validation layers off)
make release run
```

> **Windows Note**: You can invoke `mingw32-make` directly from `cmd.exe` or PowerShell (or alias `make` to `mingw32-make`). All targets and flags work identically.

---

## Starter Guide: Making It Your Project

This repository is built specifically to serve as your **starting canvas**. The build system is designed as a zero-maintenance engine: as you add, rename, or reorganize files, it automatically discovers and compiles them without requiring you to manage Makefile rules.

### 1. Renaming Your Application
By default, the executable takes the name of your root directory. To specify an explicit name and version:
1. Open [`config.mk`](config.mk).
2. Set your application name and version:
   ```makefile
   PROJECT = my-vulkan-engine
   VERSION = 1.0.0
   ```
3. Rebuild with `make rebuild`. Your binary will now be `build/debug/bin/my-vulkan-engine.exe`.

### 2. Adding Your Own Source Code
You do **not** need to touch the build system when writing new code:
* **Add new `.cpp` files** directly in `src/` or any nested directory (e.g. `src/renderer/camera.cpp`, `src/scene/mesh.cpp`). The build system recursively scans `src/` and compiles every file.
* **Add new `.hpp` files** in `include/` or alongside your code in `src/`. All subdirectories under `include/` and `src/` are automatically added to the include path (`-I`), so you can write:
  ```cpp
  #include "renderer/camera.hpp"
  ```
* **Extend or replace `VulkanApp`**: The starter class in [`src/vulkan_app.cpp`](src/vulkan_app.cpp) and [`include/vulkan_app.hpp`](include/vulkan_app.hpp) provides a full working Vulkan boilerplate (window, surface, swapchain, render pass, command buffers, and synchronization). You can freely refactor it, split it into smaller subsystems (e.g. `Window`, `Device`, `Renderer`, `PipelineCache`), or build your engine on top of it.

### 3. Adding & Customizing Shaders
* Drop any new GLSL shader into [`shaders/`](shaders/) with standard extensions (`.vert`, `.frag`, `.comp`, `.geom`, `.tesc`, `.tese`, `.mesh`, `.task`, `.rgen`, `.rmiss`, `.rchit`).
* Shaders are incrementally compiled to SPIR-V bytecode (`.spv`) during `make` and automatically copied next to your binary in `build/<mode>/bin/shaders/`.
* To compile shaders independently at any time, run:
  ```bash
  make shaders
  ```

### 4. Adding Third-Party Libraries
The template features a zero-configuration drop-in architecture:
* **Header-only libraries** (e.g. GLM, Vulkan Memory Allocator `vk_mem_alloc.h`, `stb_image.h`, `tinyobjloader`):
  Drop them into [`include/`](include/) or [`include/vendor/`](include/). They are immediately visible to your code.
* **Precompiled static or import libraries** (`.a`, `.lib`, `.dll.a`):
  Drop them into [`lib/`](lib/). The build system automatically detects and links them using order-independent groups (`--start-group ... --end-group`).
* **Runtime DLLs / shared objects** (`.dll`, `.so`, `.dylib`):
  Drop them into [`bin/`](bin/) or [`lib/`](lib/). They are automatically copied next to your compiled executable in `build/<mode>/bin/`.
* **System packages**:
  To link libraries installed via `pkg-config`, simply append them to `PKG_CONFIG_PACKAGES` in [`config.mk`](config.mk):
  ```makefile
  PKG_CONFIG_PACKAGES ?= vulkan glfw3 glm
  ```

### 5. Adding Unit & Integration Tests
* Create a new file in [`tests/`](tests/) prefixed with `test_` (e.g., `tests/test_camera.cpp`, `tests/test_math.cpp`).
* Each test file gets its own executable linked against your project code (excluding `src/main.cpp`).
* Run all tests with:
  ```bash
  make test
  ```

### 6. Building for Production / Release
* Run `make release` to produce a fully optimized binary (`-O2 -DNDEBUG -s`) with Vulkan validation layers automatically stripped.
* Output binaries and all required runtime assets are self-contained in `build/release/bin/`.

---

## Key Features

- **Pragmatic C++23 Design**:
  Clean, deterministic RAII without heavy OOP bloat or complex template metaprogramming. Uses standard library containers (`std::vector`, `std::array`, `std::span`), zero-allocation string views (`std::string_view`), optional states (`std::optional`), and type-safe formatted I/O (`std::format`).
- **Complete Working Vulkan Engine Baseline**:
  GLFW windowing with `GLFW_NO_API`, Vulkan surface, physical GPU rating (discrete GPU preference), logical device and queue retrieval, swapchain management, render pass configuration, dynamic viewport and scissor states, bufferless vertex rendering, double-buffering synchronization with semaphores and fences (`MAX_FRAMES_IN_FLIGHT = 2`), and robust swapchain recreation on window resize and minimization.
- **Automatic Shader Pipeline (`shaders/`)**:
  Any GLSL shader placed in `shaders/` is automatically discovered, incrementally compiled to SPIR-V (`.spv`) via Google's `glslc`, and deployed into `build/<mode>/bin/shaders/` alongside your executable.
- **Built-in Vulkan Validation Layers**:
  Debug builds (`BUILD=debug`) automatically enable `VK_LAYER_KHRONOS_validation` and attach a formatted debug messenger callback. Release builds (`BUILD=release`) compile with `-O2 -DNDEBUG -s` and strip validation layers for zero overhead.
- **Single Configuration Point (`config.mk`)**:
  All project settings live in `config.mk`. Automatically detects LunarG Vulkan SDK environment variables (`VULKAN_SDK`), uses `pkg-config` (`vulkan glfw3`), and provides reliable platform-specific fallback flags.
- **Zero-Friction Drop-In Architecture (`include/`, `lib/`, `bin/`)**:
  Drop headers into `include/`, static/import libraries into `lib/`, and runtime shared libraries (`.dll`, `.so`, `.dylib`) into `bin/`. The build system automatically indexes, links, and deploys them.
- **Dual Build Modes**:
  Clean separation of debug (`build/debug/`) and release (`build/release/`) artifacts.
- **Headless Vulkan Testing (`make test`)**:
  Built-in test suite (`tests/test_vulkan.cpp`) verifies GLFW Vulkan capability, `VkInstance` creation and destruction, GPU enumeration, and SPIR-V bytecode headers without requiring an interactive window.
- **Dynamic `compile_commands.json` Generation**:
  Accurate compilation database generated directly from Makefile flags on every build, providing **clangd** and **clang-tidy** perfect code intelligence with zero configuration drift.
- **Turnkey VS Code Integration**:
  Pre-configured tasks (`tasks.json`), F5 debugging with GDB/LLDB pretty-printing (`launch.json`), clangd language server (`settings.json`), and recommended GLSL extensions (`slevesque.shader`).
- **Cross-Platform CI Matrix**:
  Multi-platform GitHub Actions workflow covering Linux (GCC & Clang with software Lavapipe rasterization), macOS (Clang & MoltenVK), Windows (MSYS2 UCRT64), Sanitizers, and formatting checks.

---

## Project Structure

Each directory contains a dedicated `README.md` detailing its conventions, discovery rules, and best practices:

```text
.
├── config.mk                 # Project identity, C++23 flags, and configuration (edit this for your project)
├── Makefile                  # Portable C++ build engine (auto-discovers your sources, headers, and shaders)
├── compile_commands.json     # Clang compilation database (auto-generated on build)
├── .clang-format             # Code formatting rules (LLVM-based, tabs, clean style)
├── .clang-tidy               # Static analysis rules tailored for modern C++23
├── .gitattributes            # Line endings and binary file handling
├── .gitignore                # Ignores build outputs, caches, and system files
├── LICENSE                   # MIT License
│
├── src/                      # [README] Application C++ source code (add your .cpp files here)
│   ├── main.cpp              # Entry point: argument parsing and lifecycle coordination
│   ├── vulkan_app.cpp        # VulkanApp starter implementation (Pragmatic C++23 RAII)
│   └── README.md             # Guide: source organization and engine lifecycle
│
├── include/                  # [README] Public interface headers (add your .hpp and vendor headers here)
│   ├── vulkan_app.hpp        # VulkanApp class declaration and state
│   └── README.md             # Guide: header inclusion, API contracts, C++ library drop-ins
│
├── shaders/                  # [README] GLSL shader source files (add .vert, .frag, .comp here)
│   ├── triangle.vert         # Vertex shader: bufferless triangle positions and RGB colors
│   ├── triangle.frag         # Fragment shader: color interpolation output
│   └── README.md             # Guide: shader stages, glslc compilation, SPIR-V bytecode
│
├── lib/                      # [README] Drop-in static & import libraries (.a, .lib, .dll.a, .so, .dylib)
│   └── README.md             # Guide: auto-linking, smart selection, order-independent linking
│
├── bin/                      # [README] Drop-in runtime dynamic libraries (.dll, .so, .dylib)
│   └── README.md             # Guide: automatic deployment to build/bin, rpath handling
│
├── tests/                    # [README] Standalone unit test suites (add test_*.cpp files here)
│   ├── test_framework.hpp    # Minimal, dependency-free C++23 test assertion macros
│   ├── test_vulkan.cpp       # Headless Vulkan capability & shader verification test
│   └── README.md             # Guide: writing tests, running suites, custom frameworks
│
├── build/                    # Generated automatically during compilation (ignored by git)
│   ├── debug/                # Debug artifacts (-g3 -O0 -DDEBUG, validation layers)
│   │   ├── bin/              # Executables, test runners, shaders/, and copied runtime DLLs
│   │   └── obj/              # Object (.o) and dependency (.d) files
│   └── release/              # Release artifacts (-O2 -DNDEBUG -s, optimized)
│       ├── bin/
│       └── obj/
│
├── .vscode/                  # [README] Turnkey Visual Studio Code configuration
│   ├── extensions.json       # Recommended extensions (clangd, cpptools, makefile-tools, shader)
│   ├── launch.json           # F5 debug launcher (GDB/LLDB, integrated & external console)
│   ├── settings.json         # clangd language server, format-on-save, search exclusions, shader associations
│   ├── tasks.json            # Build, test, shaders, clean, format, tidy, and configure tasks
│   └── README.md             # Guide: VS Code tasks, debugging profiles, MSYS2 PATH injection
│
└── .github/                  # GitHub Actions configuration
    └── workflows/            # [README] Multi-OS matrix CI
        ├── ci.yml            # Workflow definition (Linux, macOS, Windows UCRT64, ASan/UBSan)
        └── README.md         # Guide: CI matrix, reproducible local checks, pull request gate
```

---

## Prerequisites & Toolchain Setup

### Windows (MSYS2 UCRT64 — Recommended)

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal and install the required tools:
   ```bash
   pacman -S --needed \
       mingw-w64-ucrt-x86_64-gcc \
       mingw-w64-ucrt-x86_64-gdb \
       mingw-w64-ucrt-x86_64-make \
       mingw-w64-ucrt-x86_64-vulkan-loader \
       mingw-w64-ucrt-x86_64-vulkan-headers \
       mingw-w64-ucrt-x86_64-shaderc \
       mingw-w64-ucrt-x86_64-glfw \
       mingw-w64-ucrt-x86_64-clang-tools-extra
   ```
3. Add `C:\msys64\ucrt64\bin` and `C:\msys64\usr\bin` to your Windows `PATH` environment variable.
4. On Windows command prompts, you can invoke `mingw32-make` directly (or alias `make` to `mingw32-make`).

> [!NOTE]
> **LunarG Vulkan SDK Support**: If you install the official [LunarG Vulkan SDK](https://vulkan.lunarg.com/), the build system automatically detects the `VULKAN_SDK` environment variable and includes its headers and library paths.

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install build-essential g++ gdb clangd clang-format clang-tidy \
                 libvulkan-dev vulkan-tools libglfw3-dev glslc mesa-vulkan-drivers
```

### Linux (Arch / Manjaro)

```bash
sudo pacman -S base-devel gcc gdb clang vulkan-devel glfw shaderc
```

### macOS (Homebrew / MoltenVK)

```bash
brew install make llvm pkg-config vulkan-headers vulkan-loader molten-vk glfw shaderc
```

---

## Configuration Guide (`config.mk`)

All project settings are centralized in [`config.mk`](config.mk). The generic [`Makefile`](Makefile) reads these variables and applies them across all recipes. Every variable uses `?=` so it can be overridden from the command line (e.g., `make CXX=clang++ BUILD=release`).

### Configuration Variables Reference

| Variable | Default Value | Description |
|---|---|---|
| `PROJECT` | Current folder name | Output binary name. Spaces are automatically converted to underscores. |
| `VERSION` | `0.1.0` | Injected into preprocessor as `PROJECT_VERSION=\"...\"`. |
| `PROJECT_TYPE` | `executable` | Target type: `executable`, `static`, or `shared`. |
| `BUILD` | `debug` | Default build mode: `debug` (validation layers on) or `release` (validation layers off). |
| `CXX` | `g++` | C++ compiler (`g++`, `clang++`, or cross-compilation triple). |
| `CXX_STANDARD` | `c++23` | C++ language standard: `c++23`, `c++20`, `c++17`. |
| `WARNINGS` | `-Wall -Wextra ...` | Strict compilation warning flags. |
| `WERROR` | `0` | When set to `1`, treats all warnings as fatal errors (recommended for CI). |
| `DEBUG_CXXFLAGS` | `-g3 -O0 -DDEBUG` | Applied in debug mode (activates validation layers). |
| `RELEASE_CXXFLAGS` | `-O2 -DNDEBUG` | Applied in release mode (optimizations enabled). |
| `GLSLC` | `glslc` | Path to the Google `glslc` shader compiler. |
| `SHADER_DIR` | `shaders` | Directory containing GLSL shader source files. |
| `PKG_CONFIG_PACKAGES` | `vulkan glfw3` | Packages queried via `pkg-config`. |
| `LDLIBS` | Platform-specific | Fallback system libraries (`-lvulkan-1 -lglfw3 -lgdi32` on Windows). |
| `SRC_DIR` | `src` | Directory containing application source code. |
| `INC_DIR` | `include` | Directory containing public header files. |
| `LIB_DIR` | `lib` | Directory containing drop-in static and import libraries. |
| `BIN_DIR` | `bin` | Directory containing drop-in runtime shared libraries. |
| `TEST_DIR` | `tests` | Directory containing unit test suites (`test_*.cpp`). |
| `BUILD_DIR` | `build` | Output directory for all compiled object and binary files. |

---

## Architecture Overview

The starter engine strikes a pragmatic balance for graphics programming:

1. **RAII Resource Management**:
   The `VulkanApp` class owns all Vulkan and GLFW handles. Its destructor deterministically waits for GPU idle and destroys all resources in reverse creation order.
2. **Standard Containers**:
   `std::vector` manages dynamic swapchain images, image views, and framebuffers without raw `malloc`/`free`.
3. **`std::span` Interface**:
   Array slices and buffer views are passed cleanly without pointer/length decoupling.
4. **Fast, Standard Vulkan C Headers**:
   Uses standard `<vulkan/vulkan.h>` instead of heavy C++ template bindings (`vulkan.hpp`), keeping build times fast while enjoying modern C++ language conveniences.

```mermaid
flowchart TD
    subgraph Initialization["1. Initialization"]
        GLFW["glfwInit & glfwCreateWindow<br/>(GLFW_NO_API)"] --> INST["VkInstance<br/>(+ Validation Layers in Debug)"]
        INST --> SURF["glfwCreateWindowSurface<br/>(VkSurfaceKHR)"]
        SURF --> GPU["Pick Physical GPU<br/>(Discrete GPU preference)"]
        GPU --> DEV["VkDevice & Queues<br/>(Graphics + Present)"]
    end

    subgraph PipelineSetup["2. Pipeline Setup"]
        DEV --> SWAP["VkSwapchainKHR & VkImageViews<br/>(B8G8R8A8_SRGB, Double/Triple Buffer)"]
        SWAP --> RP["VkRenderPass<br/>(Clear Color + Present Layout)"]
        DEV --> SHADERS["Compile Shaders to SPIR-V<br/>(glslc triangle.vert & triangle.frag)"]
        SHADERS --> PIPE["VkPipeline<br/>(Dynamic Viewport & Scissor)"]
        RP --> FB["VkFramebuffers<br/>(One per Swapchain Image)"]
        DEV --> SYNC["VkSemaphore & VkFence<br/>(MAX_FRAMES_IN_FLIGHT = 2)"]
    end

    subgraph MainLoop["3. Render Loop"]
        POLL["glfwPollEvents()"] --> ACQ["vkAcquireNextImageKHR"]
        ACQ --> REC["Record Command Buffer<br/>(Dynamic Viewport/Scissor, Draw 3 Verts)"]
        REC --> SUB["vkQueueSubmit<br/>(Wait Image Available, Signal Render Finished)"]
        SUB --> PRES["vkQueuePresentKHR<br/>(Handle Window Resize / Out-of-date)"]
        PRES --> POLL
    end

    Initialization --> PipelineSetup --> MainLoop
```

---

## Automatic Shader Pipeline (`shaders/`)

Vulkan executes SPIR-V intermediate bytecode rather than raw GLSL source text. The template provides fully automated shader discovery and compilation:

1. **Drop-in GLSL**: Place `.vert`, `.frag`, or `.comp` files directly into `shaders/`.
2. **Incremental Compilation**: The Makefile compiles only modified shaders into `build/<mode>/bin/shaders/<name>.<stage>.spv` using `glslc`.
3. **Runtime Loading**: Shaders are co-located next to the produced binary and can also be loaded relative to the project root.
4. **Bufferless Vertex Generation**: The starter vertex shader generates triangle vertices directly from `gl_VertexIndex`, avoiding unnecessary vertex buffer setup for Hello Triangle.

---

## Drop-In Dependency Architecture

Adding third-party libraries and code requires **zero Makefile modifications**:

- **Headers (`include/` and `src/`)**: Every subdirectory is automatically added to the include path (`-I`).
- **Static & Import Libraries (`lib/`)**: Discovers `.a`, `.lib`, `.dll.a`, `.so`, and `.dylib`. Auto-links with order-independent groups (`-Wl,--start-group ... -Wl,--end-group`).
- **Runtime Shared Libraries (`bin/`)**: Any `.dll`, `.so`, or `.dylib` in `bin/` or `lib/` is automatically copied into `build/<mode>/bin/` alongside your executable.

---

## Unit & Headless Testing System

The template features a standalone, modular testing system in `tests/`:

```bash
make test
```

### Architecture
- **Suite Isolation**: Every file named `tests/test_*.cpp` is compiled into an isolated executable in `build/<mode>/bin/test_<name>[.exe]`.
- **Application Logic Included**: Test suites are automatically linked against all project source objects in `src/` (excluding `src/main.cpp`), granting direct access to internal engine modules.
- **Vulkan Verification (`test_vulkan.cpp`)**: Validates GLFW Vulkan support, `VkInstance` creation and destruction, GPU enumeration, and SPIR-V bytecode magic numbers (`0x07230203`).

---

## Code Quality & Tooling

### Clang-Format (`make format` / `make format-check`)

Format all project sources and headers according to [`.clang-format`](.clang-format):
```bash
make format
```

Check code formatting without modifying files (used in CI):
```bash
make format-check
```

### Clang-Tidy (`make tidy`)

Run in-depth static analysis across all project sources using [`.clang-tidy`](.clang-tidy):
```bash
make tidy
```

### Automatic `compile_commands.json`

Every time `make` is invoked, it writes an exact compilation database (`compile_commands.json`) containing the exact compiler flags, macros, and include paths for every source file.
- **clangd** uses this file for instantaneous, precise code intelligence, jump-to-definition, and refactoring.
- **clang-tidy** uses this file to resolve includes and language dialects during analysis.

---

## Visual Studio Code Integration

Open the workspace folder in VS Code (`code .`):

- **IntelliSense**: Powered by the recommended **clangd** extension via `compile_commands.json`.
- **Shader Highlighting**: Powered by the recommended **Shader languages support** extension (`slevesque.shader`) with pre-configured `.vert`, `.frag`, and `.comp` associations.
- **Keyboard Shortcuts**:
  - `Ctrl + Shift + B`: Run default build task (compiles project and shaders).
  - `F5`: Build and launch with GDB / LLDB debugger.
  - `Ctrl + F5`: Run without debugging.
- **Tasks (`Terminal > Run Task...`)**:
  - `Build`, `Build Release`, `Compile Shaders`, `Run`, `Test`, `Clean`, `Rebuild`, `Format`, `Tidy`, `Configure`.

See [`.vscode/README.md`](.vscode/README.md) for complete details.

---

## Continuous Integration (GitHub Actions)

The repository includes a multi-platform CI workflow in [`.github/workflows/ci.yml`](.github/workflows/ci.yml):

- **Ubuntu (GCC & Clang)**: Debug build, shader compilation, headless test run via `xvfb-run` and Mesa Lavapipe software rasterization, AddressSanitizer + UndefinedBehaviorSanitizer run, Release build.
- **macOS (Clang)**: Debug build, shader compilation, MoltenVK linkage, Release build.
- **Windows (MSYS2 UCRT64)**: Debug build, shader compilation, headless test run with Mesa Lavapipe, Release build.
- **Clang-Format Gate**: Verifies style compliance across all files.

See [`.github/workflows/README.md`](.github/workflows/README.md) for local verification steps.

---

## Make Targets Reference

| Target | Description | Example Command |
|---|---|---|
| `all` *(default)* | Build application and compile all shaders in debug mode | `make` |
| `release` | Build optimized release binary without validation layers | `make release` |
| `shaders` | Compile GLSL shaders in `shaders/` to SPIR-V | `make shaders` |
| `run` | Build and execute the application | `make run ARGS="--help"` |
| `test` | Build and run all test suites in `tests/` | `make test` |
| `clean` | Delete the `build/` directory | `make clean` |
| `rebuild` | Clean and rebuild everything | `make rebuild` |
| `format` | Format all source and header files | `make format` |
| `format-check` | Check formatting compliance without editing | `make format-check` |
| `tidy` | Run `clang-tidy` static analysis | `make tidy` |
| `info` | Display detected toolchains, flags, shaders, and libraries | `make info` |
| `help` | Print Makefile usage and variable summary | `make help` |
| `run-<test_name>` | Build and execute a single test suite | `make run-test_vulkan` |

---

## Troubleshooting & FAQ

### 1. `error: Vulkan is not supported by the local graphics driver`
Ensure your graphics driver is up to date. On Windows, NVIDIA, AMD, and Intel provide Vulkan drivers in their standard driver packages. On virtual machines or headless servers without a physical GPU, install a software rasterizer such as SwiftShader or Mesa Lavapipe.

### 2. Validation layer warnings or missing layer
In debug builds, `VK_LAYER_KHRONOS_validation` is enabled by default. If the layer is not installed, the application prints a warning and proceeds without validation. To install validation layers:
* **Windows**: Install the LunarG Vulkan SDK or run `pacman -S mingw-w64-ucrt-x86_64-vulkan-validation-layers` in MSYS2.
* **Linux**: Run `sudo apt install vulkan-validationlayers`.
* **macOS**: Included with the LunarG macOS SDK or MoltenVK.

### 3. `glslc: command not found`
The shader compiler `glslc` is part of the Vulkan SDK and the `shaderc` package:
* **Windows**: Included with the LunarG Vulkan SDK or via MSYS2 `pacman -S mingw-w64-ucrt-x86_64-shaderc`.
* **Linux**: `sudo apt install glslc` or `shaderc`.
* **macOS**: `brew install shaderc`.

### 4. `make: command not found` on Windows
Windows does not ship with `make` by default. If you installed MSYS2 UCRT64:
* Run `mingw32-make` instead of `make`.
* Or alias `make` to `mingw32-make` in PowerShell / cmd.

---

## License

This project template is licensed under the [MIT License](LICENSE). You are free to use, modify, and distribute it in both open-source and proprietary software.
