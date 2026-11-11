# `tests/` — Vulkan Unit & Integration Testing System (Pragmatic C++23)

This directory contains standalone unit test suites and test helper utilities.

The template includes a fast, modular test runner invoked with:
```bash
make test
```

---

## How It Works

1. **Automated Test Suite Discovery**:
   - Every file matching `tests/test_*.cpp` (e.g., `test_vulkan.cpp`, `test_math.cpp`) is automatically recognized as an independent test suite.
   - Each test file is compiled into its own isolated executable:
     ```text
     build/<mode>/bin/test_<name>[.exe]
     ```

2. **Clean Separation from `main.cpp`**:
   - Test executables link against all project source files in `src/` **except** `src/main.cpp` (`MAIN_SRC`).
   - This ensures each test suite can define its own `int main()` without symbol collision.

3. **Test Support / Helper Files**:
   - Any `.cpp` file inside `tests/` that does **not** match `test_*.cpp` is treated as shared test support code.
   - Support code is compiled into object files and automatically linked into **every** test executable.

4. **Runtime Library & Shader Access**:
   - Test binaries are placed in `build/<mode>/bin/`, directly beside copied runtime shared libraries (`.dll`, `.so`, `.dylib`) and compiled shaders (`shaders/*.spv`).
   - Tests execute with complete access to dynamic dependencies and SPIR-V bytecode.

---

## The Built-in Test Framework (`tests/test_framework.hpp`)

The template includes a lightweight, zero-dependency C++23 testing helper:

### Available Assertion Macros

| Macro | Description | Failure Behavior |
|---|---|---|
| `CHECK(cond)` | Verifies that boolean expression `cond` is true. | Prints filename, line number, and expression text. |
| `CHECK_EQ(expected, actual)` | Compares two expressions for equality (`==`). | Prints expected vs. actual values on mismatch. |

### Test Runner & Reporting Macros

| Macro | Description |
|---|---|
| `RUN_TEST(fn)` | Executes a `void fn()` test function, tracking failures and printing `PASS <fn>` or `FAIL <fn>`. |
| `TEST_REPORT()` | Flushes stdout, prints total test and check counts, and returns `0` on success or `1` on failure. |

---

## Included Suite: `tests/test_vulkan.cpp`

The template provides a comprehensive headless integration test verifying your graphics environment:

1. **`test_glfw_vulkan_support`**:
   Validates that the local display server reports Vulkan support (`glfwVulkanSupported() == GLFW_TRUE`).
2. **`test_vulkan_instance_lifecycle`**:
   Creates and destroys a `VkInstance` to verify Vulkan loader linkage and driver communication.
3. **`test_physical_device_enumeration`**:
   Queries and logs all available physical GPUs, their Vulkan API versions, and device names using `std::format`.
4. **`test_shader_spv_magic`**:
   Reads `triangle.vert.spv` and `triangle.frag.spv` via `std::ifstream` to verify that shader bytecode exists and begins with the valid SPIR-V magic number (`0x07230203`).

### Example Output

```text
  CXX    tests/test_vulkan.cpp
  LINK   build/debug/bin/test_vulkan.exe
  TEST   test_vulkan
  PASS  test_glfw_vulkan_support
  PASS  test_vulkan_instance_lifecycle
      Device 0: AMD Radeon(TM) 780M (Vulkan 1.3.246)
      Device 1: NVIDIA GeForce RTX 4050 Laptop GPU (Vulkan 1.4.357)
  PASS  test_physical_device_enumeration
  PASS  test_shader_spv_magic
  4/4 tests passed (12 checks, 0 failed)
All tests passed: test_vulkan
```

---

## Running Individual Test Suites

Run a specific test suite directly:
```bash
make run-test_vulkan
```
