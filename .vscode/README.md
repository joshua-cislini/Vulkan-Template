# `.vscode/` — Visual Studio Code Integration

This directory contains turnkey development, build, debug, and IntelliSense configuration for **Visual Studio Code**.

---

## Configuration Files Overview

| File | Purpose | Key Features |
|---|---|---|
| [`settings.json`](settings.json) | Editor & Language Server Settings | Enables **clangd**, activates inline **clang-tidy**, sets format-on-save, configures file associations. |
| [`tasks.json`](tasks.json) | Build, Test, Clean & Tooling Tasks | 10 predefined tasks mapping directly to Makefile targets; compiles SPIR-V shaders; auto-injects MSYS2 toolchain paths on Windows. |
| [`launch.json`](launch.json) | F5 Debugger Configurations | Dual GDB/LLDB profiles (integrated console & external terminal for interactive input). |
| [`extensions.json`](extensions.json) | Extension Recommendations | Recommends `vscode-clangd`, `cpptools` (debug engine), `makefile-tools`, and `dan-c-underwood.shader` (GLSL syntax). |

---

## Detailed File Breakdown

### 1. `settings.json` — Fast, Drift-Free Language Support

This template configures **clangd** as the primary language engine:
- **IntelliSense Engine Disabled in `cpptools`**:
  `"C_Cpp.intelliSenseEngine": "disabled"` avoids running two competing language servers simultaneously.
- **Clangd Configuration**:
  - `--background-index`: Indexes your codebase in the background for instant symbol search.
  - `--clang-tidy`: Displays clang-tidy static analysis warnings and suggestions directly in your editor as you type.
  - `--header-insertion=never`: Prevents unwanted automatic `#include` additions when typing common symbols.
  - `--query-driver`: Allows clangd to discover GCC/Clang built-in system headers and Vulkan headers across MinGW, Linux, and macOS.
- **Format on Save**:
  Automatically formats C files on save using the root `.clang-format` configuration.
- **File Associations**:
  - `*.h` files are explicitly treated as C.
  - `*.mk` files (like `config.mk`) are treated as Makefiles.

---

### 2. `tasks.json` — Integrated Build and Workflow Tasks

All tasks automatically use `mingw32-make` on Windows and `make` on Linux/macOS. On Windows, the task runner prepends `C:\msys64\ucrt64\bin;C:\msys64\usr\bin` to `PATH`, ensuring builds succeed even if MSYS2 was not added to the system PATH.

#### Available Tasks (`Terminal > Run Task...`):

1. **`Build`** *(Default Build Task — `Ctrl + Shift + B`)*:
   Builds the Vulkan application and compiles all shaders (`make`).
2. **`Build Release`**:
   Builds an optimized release binary without validation layers (`make release`).
3. **`Compile Shaders`**:
   Compiles GLSL shaders in `shaders/` into SPIR-V bytecode (`make shaders`).
4. **`Run`**:
   Builds and executes the Vulkan Triangle application (`make run`).
5. **`Test`** *(Default Test Task)*:
   Builds and executes the Vulkan test suite (`make test`).
6. **`Clean`**:
   Deletes the `build/` directory (`make clean`).
7. **`Rebuild`**:
   Executes a clean followed by a full rebuild (`make rebuild`).
8. **`Format`**:
   Formats all project sources and headers with `clang-format` (`make format`).
9. **`Tidy`**:
   Executes static analysis across all files with `clang-tidy` (`make tidy`).
10. **`Configure`** *(Auto-run on Folder Open)*:
    Silently runs `make info` when VS Code opens the workspace, generating `compile_commands.json` immediately so clangd is active before your first manual build.

All compilation tasks use the `$gcc` problem matcher so errors and warnings appear directly in the **Problems** tab with clickable links to line numbers.

---

### 3. `launch.json` — Interactive Debugging

Pressing **F5** automatically runs the `Build` task and attaches the debugger to `build/debug/bin/<PROJECT>[.exe]`.

#### Configurations Provided:

1. **`Debug`**:
   - Primary debug target.
   - Runs within the VS Code integrated Debug Console.
   - Enables GDB pretty-printing (`-enable-pretty-printing`) for inspecting complex structs, strings, and arrays.
   - On macOS, automatically selects `lldb`. On Windows, targets `C:\msys64\ucrt64\bin\gdb.exe`.
2. **`Debug (external console)`**:
   - Launches your program in a standalone terminal window (`externalConsole: true`).
   - Use this configuration whenever debugging interactive CLI applications or when running fullscreen/special window modes.

---

### 4. `extensions.json` — Recommended Extensions

When you open this project in VS Code, you will be prompted to install:
- **`llvm-vs-code-extensions.vscode-clangd`**: High-performance C/C++ language server.
- **`ms-vscode.cpptools`**: Microsoft C/C++ extension (used exclusively as the debug adapter for GDB/LLDB).
- **`ms-vscode.makefile-tools`**: Makefile syntax highlighting and target exploration.
- **`dan-c-underwood.shader`**: Syntax highlighting for `.vert`, `.frag`, and `.comp` GLSL shaders.

---

## Troubleshooting VS Code Integration

### Missing Toolchain on Windows
If GDB or Make cannot be found:
1. Ensure MSYS2 is installed at `C:\msys64`.
2. In the MSYS2 UCRT64 terminal, run:
   ```bash
   pacman -S --needed \
       mingw-w64-ucrt-x86_64-gcc \
       mingw-w64-ucrt-x86_64-gdb \
       mingw-w64-ucrt-x86_64-make \
       mingw-w64-ucrt-x86_64-vulkan-loader \
       mingw-w64-ucrt-x86_64-vulkan-headers \
       mingw-w64-ucrt-x86_64-shaderc \
       mingw-w64-ucrt-x86_64-glfw
   ```
