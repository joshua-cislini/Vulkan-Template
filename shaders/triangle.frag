#version 450

// Interpolated color received from the vertex shader
layout(location = 0) in vec3 fragColor;

// Final output color written to the framebuffer color attachment (location 0)
layout(location = 0) out vec4 outColor;

// Push constant containing elapsed time updated from the CPU each frame
layout(push_constant) uniform PushConstants {
	float time;
} pc;

void main() {
	// Gently animate color channels over time using out-of-phase sine waves
	vec3 animatedColor = fragColor * (0.6 + 0.4 * sin(pc.time * 2.0 + vec3(0.0, 2.094, 4.188)));
	outColor = vec4(animatedColor, 1.0);
}
