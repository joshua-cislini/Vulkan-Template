# `lib/` — Drop-In Static and Import Libraries

This directory is designed for third-party compiled libraries. Drop pre-compiled static libraries or shared library import files here—the build system automatically discovers, indexes, and links them with zero configuration.

---

## Supported Library Formats

The build system recognizes the following file extensions across platforms:

| Platform | Static Libraries | Import / Shared Libraries |
|---|---|---|
| **Windows** | `.a`, `.lib` | `.dll.a`, `.lib` |
| **Linux** | `.a` | `.so` |
| **macOS** | `.a` | `.dylib` |

---

## How It Works

1. **Recursive Library Search**:
   - The Makefile recursively inspects `lib/` and all subdirectories (excluding `cmake` and `pkgconfig` metadata folders).
   - All detected library directories are added to the linker's search path (`-Llib`, `-Llib/subfolder`, etc.).

2. **Zero-Configuration Auto-Linking (`AUTO_LINK = 1`)**:
   - By default, all libraries detected in `lib/` are automatically passed to the linker.
   - **Order-Independent Linking**: On Windows (MinGW) and Linux, libraries are wrapped in:
     ```text
     -Wl,--start-group <libraries> -Wl,--end-group
     ```
     This resolves mutual dependencies, circular references, and out-of-order symbols automatically without needing to manually order linker flags.
   - **Smart Library Deduplication**:
     If a library is provided as both a static archive and a shared/import library (e.g., `libglfw3.a` and `libglfw3.dll.a`), the build system automatically prefers the shared/import library to avoid missing transitive system symbols.
   - **Dead-Stripping with `--as-needed`**:
     The linker is invoked with `-Wl,--as-needed`, ensuring that shared libraries dropped into `lib/` are only linked into the final binary if symbols from them are actually referenced in your code.
   - **Linux Exact Linking**:
     On Linux, libraries are linked via `-l:<filename>` (e.g., `-l:libcustom.so`) to prevent embedding hard-coded absolute machine paths in the output binary.

3. **Automatic Runtime Copying**:
   - If you drop shared libraries (`.dll`, `.so`, `.dylib`) directly into `lib/`, the build system detects them as runtime dependencies and automatically copies them to `build/<mode>/bin/` alongside your executable. (You can also place runtime libraries directly into `bin/`).

---

## Configuration Options in `config.mk`

### Excluding Specific Libraries (`AUTO_LINK_EXCLUDE`)
If `lib/` contains test archives, debug variants, or alternative builds that should not be linked into the main application, exclude them by name or pattern:
```make
AUTO_LINK_EXCLUDE = libglfw3_debug.a %_static.lib
```

### Manual Linking (`AUTO_LINK = 0`)
To take manual control over library linking order and flags:
```make
AUTO_LINK = 0
LDLIBS    = -Llib -lvulkan-1 -lglfw3 -lgdi32 -lm
```

### Distinguishing System Libraries from Drop-In Libraries
- **Drop-in libraries** (third-party `.a`, `.lib`, `.dll.a`) go into `lib/`.
- **System / OS libraries** provided by your compiler toolchain (e.g., `-lvulkan-1`, `-lglfw3`, `-lgdi32` on Windows; `-lvulkan`, `-lglfw`, `-lm` on Linux/macOS) do **not** need to go into `lib/`. Instead, they are resolved via `PKG_CONFIG_PACKAGES ?= vulkan glfw3` or the platform `LDLIBS` fallback in `config.mk`.

---

## Verifying Discovered Libraries

Run the following command at any time to verify which libraries the build system has detected and will link:
```bash
make info
```
Look for the `Library dirs:` and `Auto-linked:` lines in the output.

---

## Version Control Considerations

- The directory is preserved in version control via this `README.md` file.
- By default, `.gitignore` allows files inside `lib/` to be tracked.
- If your project uses large binary files that you prefer not to commit to Git, uncomment the following lines in `.gitignore`:
  ```gitignore
  # lib/*
  # !lib/README.md
  ```
