#version 450

// Interpolated color received from the vertex shader
layout(location = 0) in vec3 fragColor;

// Final output color written to the framebuffer color attachment (location 0)
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
