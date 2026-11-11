# `bin/` — Drop-In Runtime Shared Libraries

This directory is designed for runtime dynamic libraries required by your application or third-party dependencies when executing.

---

## Supported Runtime Formats

| Platform | File Formats |
|---|---|
| **Windows** | `.dll` |
| **Linux** | `.so`, `.so.*` |
| **macOS** | `.dylib` |

---

## How It Works

1. **Automatic Deployment to Build Output**:
   - The Makefile recursively scans `bin/` (and `lib/`) for runtime libraries.
   - On every build (`make`, `make release`, or `make test`), any detected runtime library is automatically copied into the corresponding output directory:
     - `build/debug/bin/` (debug builds)
     - `build/release/bin/` (release builds)
   - This places runtime libraries directly next to the produced application executable and unit test binaries.

2. **Zero-Hassle Execution**:
   - **Windows**: Windows searches the directory of the executing `.exe` first. By copying DLLs directly alongside your binary, the program runs immediately without needing to alter system `PATH` or manually copy DLLs.
   - **Linux**: The Makefile compiles binaries with `-Wl,-rpath,'$$ORIGIN'`, instructing the runtime dynamic loader (`ld.so`) to check the binary's directory for `.so` files first.
   - **macOS**: The Makefile compiles binaries with `-Wl,-rpath,@executable_path`, allowing macOS `dyld` to locate `.dylib` files residing in the same directory.

3. **Seamless Test Suite Support**:
   - Unit tests running under `make test` execute from `build/<mode>/bin/`.
   - Because runtime dependencies are automatically deployed into that folder, tests can call functions in shared libraries without dynamic linker errors.

---

## `bin/` vs. `lib/`: What Goes Where?

| Directory | Content Type | Role in Build System | Examples |
|---|---|---|---|
| `bin/` | **Runtime Dynamic Libraries** | Copied alongside executables; **never** passed to the linker command line. | `glfw3.dll`, `vulkan-1.dll`, `libMoltenVK.dylib` |
| `lib/` | **Link-Time Libraries** | Passed to the linker (`-L`, `-l`, `AUTO_LINK`); shared objects are also copied to `build/.../bin/`. | `libglfw3.a`, `libglfw3.dll.a`, `libvulkan-1.dll.a` |

> [!TIP]
> If you have a Windows library package containing both a link-time import library (e.g. `glfw3dll.a` or `glfw3.lib`) and a runtime dynamic link library (`glfw3.dll`), place the `.dll.a`/`.lib` into `lib/` and the `.dll` into `bin/`.

---

## Verifying Runtime Files

Run the following command to check which runtime files the build system has identified:
```bash
make info
```
Look for the `Runtime files  :` line in the output.

---

## Version Control

- The directory is preserved in version control via this `README.md` file.
- By default, `.gitignore` allows files in `bin/` to be committed.
- If you prefer not to track large binaries in Git (e.g., if you fetch them with a setup script), uncomment the following lines in `.gitignore`:
  ```gitignore
  # bin/*
  # !bin/README.md
  ```
