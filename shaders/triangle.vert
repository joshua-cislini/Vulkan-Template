#version 450

// Output color to fragment shader (location 0 matches fragment input location 0)
layout(location = 0) out vec3 fragColor;

// Hardcoded triangle vertex positions in Vulkan Normalized Device Coordinates (NDC)
// Vulkan NDC:
//   X: [-1.0, 1.0] (left to right)
//   Y: [-1.0, 1.0] (top to bottom, inverted compared to OpenGL)
//   Z: [ 0.0, 1.0] (near to far)
const vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // Top vertex (pointing up)
    vec2(0.5, 0.5),    // Bottom-right vertex
    vec2(-0.5, 0.5)    // Bottom-left vertex
);

// Per-vertex colors: Red, Green, Blue
const vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0), // Top: Red
    vec3(0.0, 1.0, 0.0), // Bottom-right: Green
    vec3(0.0, 0.0, 1.0)  // Bottom-left: Blue
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
