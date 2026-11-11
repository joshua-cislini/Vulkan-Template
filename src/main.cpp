#include "vulkan_app.hpp"

#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>

#ifndef PROJECT_NAME
#define PROJECT_NAME "Vulkan Triangle"
#endif

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.1.0"
#endif

int main(int argc, char *argv[]) {
	if(argc > 1) {
		std::string_view arg = argv[1];
		if(arg == "--version" || arg == "-v") {
			std::cout << std::format("{} {}\n", PROJECT_NAME, PROJECT_VERSION);
			return EXIT_SUCCESS;
		}
		if(arg == "--help" || arg == "-h") {
			std::cout << std::format("Usage: {} [options]\n\n", argv[0])
					  << "Options:\n"
					  << "  -h, --help     Show this help message and exit\n"
					  << "  -v, --version  Show version information and exit\n";
			return EXIT_SUCCESS;
		}
	}

	VulkanApp app;
	if(!app.init(800, 600, PROJECT_NAME " - Vulkan Triangle")) {
		std::cerr << "error: Failed to initialize Vulkan application.\n";
		return EXIT_FAILURE;
	}

	app.run();
	return EXIT_SUCCESS;
}
