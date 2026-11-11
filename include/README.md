# `include/` — Public Interface and Headers (Pragmatic C++23)

This directory contains public C++ interface header files (`.hpp`) for the project, as well as headers for drop-in third-party libraries.

---

## How It Works

1. **Recursive Include Path Discovery**:
   - The build system (`Makefile`) recursively discovers `include/` and **all nested subdirectories** (e.g., `include/`, `include/render/`, `include/vulkan/`).
   - Every detected directory is automatically prepended to the compiler command with `-I<path>`.
   - You can include headers placed directly in `include/` or nested folders without needing complex relative paths:
     ```cpp
     #include "vulkan_app.hpp"
     #include "math/matrix.hpp"
     ```

2. **Language Server & Tooling Sync**:
   - The automatically generated `compile_commands.json` receives the full list of `-I` search paths on every build.
   - **clangd** and **VS Code IntelliSense** immediately recognize all headers, providing full autocompletion, hover documentation, and diagnostics without manual JSON configuration.

---

## Included Interface: `include/vulkan_app.hpp`

`vulkan_app.hpp` defines the public `VulkanApp` class coordinating the Vulkan Triangle lifecycle:

```cpp
#include "vulkan_app.hpp"

int main() {
    VulkanApp app;
    if (!app.init(800, 600, "Vulkan Window")) {
        return 1;
    }

    app.run();
    // Destructor automatically cleans up all resources on return
    return 0;
}
```

### Key Components

- **`VulkanApp` Class**: Manages all GLFW and Vulkan handles (`VkInstance`, `VkDevice`, `VkSwapchainKHR`, `VkPipeline`, `VkRenderPass`, `VkCommandPool`, etc.).
- **`MAX_FRAMES_IN_FLIGHT`**: Constant set to `2` for CPU/GPU double-buffering.
- **`init(width, height, title)`**: Initializes GLFW, surface, logical device, swapchain, render pass, shaders, and sync primitives.
- **`run()`**: Executes the main event and rendering loop with resize recreation.
- **`cleanup()` / `~VulkanApp()`**: Deterministic RAII destruction in reverse creation order.

---

## Dropping in C++ Libraries

You can drop header-only graphics, math, or utility libraries (such as [GLM](https://github.com/g-truc/glm), [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator), [ImGui](https://github.com/ocornut/imgui), or [stb](https://github.com/nothings/stb)) directly into `include/`:
- Headers are immediately available across `src/` and `tests/`.
- For single-header libraries requiring implementation macros, place the `#define` and `#include` in a dedicated `.cpp` file in `src/` (e.g., `src/vma_impl.cpp`).
