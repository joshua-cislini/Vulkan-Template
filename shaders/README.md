# `shaders/` — Vulkan GLSL & SPIR-V Shaders

This directory contains GLSL shader source files for the Vulkan rendering pipeline.

---

## Automatic Shader Compilation

The build system (`Makefile`) automatically discovers and compiles all shader sources located in `shaders/` using Google's `glslc` compiler (included with the Vulkan SDK and MSYS2).

### Supported Shader Stages

| Stage | Extension | Target Bytecode Output | Description |
|---|---|---|---|
| **Vertex** | `.vert` | `build/<mode>/bin/shaders/<name>.vert.spv` | Processes per-vertex positions, attributes, and colors |
| **Fragment** | `.frag` | `build/<mode>/bin/shaders/<name>.frag.spv` | Calculates final pixel colors output to color attachments |
| **Compute** | `.comp` | `build/<mode>/bin/shaders/<name>.comp.spv` | General-purpose GPU compute kernels |
| **Geometry** | `.geom` | `build/<mode>/bin/shaders/<name>.geom.spv` | Generates primitive geometry |
| **Tessellation** | `.tesc`, `.tese` | `build/<mode>/bin/shaders/<name>.tes*.spv` | Controls and evaluates surface subdivision |
| **Mesh / Task** | `.mesh`, `.task` | `build/<mode>/bin/shaders/<name>.mesh.spv` | Modern mesh shading pipeline |
| **Ray Tracing** | `.rgen`, `.rmiss`, `.rchit`, `.rahit` | `build/<mode>/bin/shaders/<name>.r*.spv` | Hardware ray tracing shader stages |

---

## How It Works

1. **Automatic Discovery & Incremental Builds**:
   - Any shader placed inside `shaders/` is detected automatically during `make`.
   - Recompilation is incremental: modifying a shader file recompiles only that specific shader.

2. **Deployment next to Executables**:
   - Compiled `.spv` bytecode files are placed inside `build/<mode>/bin/shaders/` alongside the executable.
   - At runtime, the application automatically locates shaders whether launched from the project root (`make run`), through VS Code (`F5`), or executed directly from `build/<mode>/bin/`.

3. **Explicit Shader Compilation Target**:
   - Run `make shaders` to recompile all shaders independently of the C source code.

---

## The Triangle Shaders

### 1. `triangle.vert` (Vertex Shader)
Uses modern bufferless vertex generation. Vertex coordinates and colors are defined directly in the shader and indexed via `gl_VertexIndex`:

```glsl
#version 450

layout(location = 0) out vec3 fragColor;

const vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // Top vertex
    vec2(0.5, 0.5),    // Bottom-right vertex
    vec2(-0.5, 0.5)    // Bottom-left vertex
);

const vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0), // Red
    vec3(0.0, 1.0, 0.0), // Green
    vec3(0.0, 0.0, 1.0)  // Blue
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
```

> [!NOTE]
> **Vulkan Normalized Device Coordinates (NDC)**:
> - **X**: `[-1.0, 1.0]` (left to right)
> - **Y**: `[-1.0, 1.0]` (top to bottom — inverted relative to OpenGL)
> - **Z**: `[ 0.0, 1.0]` (near plane to far plane)

### 2. `triangle.frag` (Fragment Shader)
Interpolates vertex colors and writes the final RGBA color to the framebuffer attachment:

```glsl
#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    float time;
} pc;

void main() {
    vec3 animatedColor = fragColor * (0.6 + 0.4 * sin(pc.time * 2.0 + vec3(0.0, 2.094, 4.188)));
    outColor = vec4(animatedColor, 1.0);
}
```

---

## Adding Your Own Shaders

The included triangle shaders are starting examples. You can freely extend or replace them:

1. **Adding 3D & Vertex Buffers**:
   Add uniform buffer objects (UBO) or push constants for Model-View-Projection matrices (e.g. `layout(binding = 0) uniform UniformBufferObject { mat4 model; mat4 view; mat4 proj; } ubo;`), and define vertex input bindings (`layout(location = 0) in vec3 inPosition;`).

2. **Adding Compute Shaders (`.comp`)**:
   Drop any file ending in `.comp` into `shaders/` (e.g., `particles.comp`). It will be compiled to `build/<mode>/bin/shaders/particles.comp.spv` on the next build.

3. **Subdirectories in `shaders/`**:
   The Makefile discovers shaders recursively, so you can organize complex shader pipelines into folders (e.g., `shaders/postprocess/bloom.frag`, `shaders/lighting/pbr.frag`).
