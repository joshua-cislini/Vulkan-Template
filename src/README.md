# `src/` — Application and Vulkan Source Code (Pragmatic C++23)

This directory contains the primary C++ source files (`.cpp`) and internal/private header files (`.hpp`) for the Vulkan project.

---

## How It Works

The build system (`Makefile`) automatically discovers and compiles code in `src/` without requiring any manual updates:

1. **Recursive Source Discovery**:
   - Every `.cpp`, `.cxx`, or `.cc` file located directly in `src/` or within any nested subdirectory (e.g., `src/core/`, `src/render/`) is automatically detected via recursive wildcard scanning.
   - Each source file is compiled into a corresponding object file in `build/<mode>/obj/src/...` preserving directory hierarchy.

2. **Automatic Internal Header Inclusion**:
   - `src/` and **all its nested subdirectories** are automatically added to the compiler's include path (`-I`).
   - Any private header placed inside `src/` can be included cleanly without convoluted relative paths (`../../`):
     ```cpp
     #include "render_internal.hpp"
     #include "pipeline_cache.hpp"
     ```

3. **Application Entry Point (`src/main.cpp`)**:
   - `src/main.cpp` contains the standard `main()` function for the Vulkan application.
   - The Makefile designates this file through `MAIN_SRC ?= $(SRC_DIR)/main.cpp`.
   - **Crucial Behavior**:
     - When building the main application (`make`), `main.cpp` is compiled and linked.
     - When building **unit tests** (`make test`), `main.cpp` is **automatically excluded** so each test suite in `tests/test_*.cpp` can provide its own `main()` function without symbol collisions.

4. **Injected Preprocessor Macros**:
   Every source file in `src/` is compiled with project metadata injected directly from `config.mk`:
   - `PROJECT_NAME`: A string literal representing the project name (e.g., `"Vulkan-Template"`).
   - `PROJECT_VERSION`: A string literal representing the current version (e.g., `"0.1.0"`).
   - `DEBUG` (in debug mode, enabling Vulkan validation layers) or `NDEBUG` (in release mode).

---

## Architecture of Included Files

```text
src/
├── main.cpp         # Entry point: argument parsing and lifecycle coordinator
├── vulkan_app.cpp   # VulkanApp class implementation (Pragmatic C++23 RAII)
└── README.md        # Guide: source organization and engine lifecycle
```

### 1. `main.cpp` — Application Entry Point
- Handles command-line arguments (`--version`, `-v`, `--help`, `-h`).
- Instantiates the `VulkanApp` class.
- Calls `app.init(800, 600, ...)` and `app.run()`.
- Deterministic cleanup is automatically performed by the `VulkanApp` destructor when exiting `main()`.

### 2. `vulkan_app.cpp` — Pragmatic C++23 Implementation
Implements the complete Vulkan rendering pipeline leveraging modern C++ features:
- **Zero-Boilerplate File I/O**: Shader binaries loaded via `std::ifstream` directly into `std::vector<char>`, automatically handling cleanup and size calculation.
- **Dynamic Array Management**: Uses `std::vector` for swapchain images, image views, and framebuffers, and `std::array` for per-frame synchronization objects.
- **Type-Safe String Formatting**: Logs messages, device properties, and validation layer output using `std::format`.
- **Validation Layers**: Attaches `VK_LAYER_KHRONOS_validation` and a formatted debug callback in debug builds (`BUILD=debug`), stripped in release builds (`BUILD=release`).
- **Device Rating**: Evaluates physical GPUs, preferring discrete GPUs over integrated hardware, querying queue families using `std::optional`.
- **Dynamic Viewport & Scissor**: Recreates swapchains on resize or minimization without recreating the entire graphics pipeline.
- **Strict Single Ownership**: Copy and move constructors/assignments are deleted to guarantee handles are never duplicated or prematurely closed.

---

## Making It Your Own: Extending & Customizing `src/`

This template is structured so you can freely modify, refactor, or replace the starter implementation as your graphics application evolves:

1. **Splitting into Subsystems**:
   As your engine grows, you can decompose `VulkanApp` into distinct classes:
   - `src/renderer/swapchain.cpp`: Swapchain creation, image views, and resize handling.
   - `src/renderer/pipeline.cpp`: Vulkan graphics/compute pipelines and shader modules.
   - `src/renderer/renderer.cpp`: Command buffer recording, draw passes, and frame synchronization.
   - `src/scene/camera.cpp`: Projection, view matrices, and input handling.

2. **Zero-Configuration Expansion**:
   Whenever you add a new `.cpp` file anywhere in `src/` (or create new subdirectories like `src/core/`, `src/math/`), the build system automatically includes and compiles it on your next `make`.

3. **Writing Tests for Your Modules**:
   Because `src/main.cpp` is excluded when running `make test`, all your code in `src/` is directly accessible and linkable to unit tests in `tests/test_*.cpp`.
