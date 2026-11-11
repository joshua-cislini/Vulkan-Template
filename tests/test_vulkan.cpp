#include "test_framework.hpp"

#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr uint32_t SPIRV_MAGIC = 0x07230203u;

static void test_glfw_vulkan_support() {
	int glfw_ok = glfwInit();
	CHECK(glfw_ok == GLFW_TRUE);

	int vk_supported = glfwVulkanSupported();
	CHECK(vk_supported == GLFW_TRUE);

	glfwTerminate();
}

static void test_vulkan_instance_lifecycle() {
	VkApplicationInfo app_info {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Vulkan C++ Test",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_0};

	VkInstanceCreateInfo create_info {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &app_info};

	VkInstance instance = VK_NULL_HANDLE;
	VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
	CHECK_EQ(VK_SUCCESS, result);
	CHECK(instance != VK_NULL_HANDLE);

	if(instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, nullptr);
	}
}

static void test_physical_device_enumeration() {
	VkApplicationInfo app_info {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_0};
	VkInstanceCreateInfo create_info {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &app_info};

	VkInstance instance = VK_NULL_HANDLE;
	VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
	CHECK_EQ(VK_SUCCESS, result);

	if(result == VK_SUCCESS) {
		uint32_t device_count = 0;
		result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		CHECK_EQ(VK_SUCCESS, result);

		if(device_count > 0) {
			std::vector<VkPhysicalDevice> devices(device_count);
			vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
			for(uint32_t i = 0; i < device_count; ++i) {
				VkPhysicalDeviceProperties props;
				vkGetPhysicalDeviceProperties(devices[i], &props);
				std::cout << std::format("      Device {}: {} (Vulkan {}.{}.{})\n",
										 i, props.deviceName,
										 VK_API_VERSION_MAJOR(props.apiVersion),
										 VK_API_VERSION_MINOR(props.apiVersion),
										 VK_API_VERSION_PATCH(props.apiVersion));
			}
		}
		vkDestroyInstance(instance, nullptr);
	}
}

static void verify_spirv_header(std::string_view path) {
	std::array<std::string, 3> candidates = {
			std::string(path),
			std::string("shaders/") + std::string(path),
			std::string("build/debug/bin/shaders/") + std::string(path)};

	std::ifstream file;
	for(const auto &p : candidates) {
		file.open(p, std::ios::binary);
		if(file.is_open()) {
			break;
		}
	}

	CHECK(file.is_open());
	if(file.is_open()) {
		uint32_t magic = 0;
		file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
		CHECK_EQ(sizeof(magic), static_cast<size_t>(file.gcount()));
		CHECK_EQ(SPIRV_MAGIC, magic);
	}
}

static void test_shader_spv_magic() {
	verify_spirv_header("triangle.vert.spv");
	verify_spirv_header("triangle.frag.spv");
}

int main() {
	RUN_TEST(test_glfw_vulkan_support);
	RUN_TEST(test_vulkan_instance_lifecycle);
	RUN_TEST(test_physical_device_enumeration);
	RUN_TEST(test_shader_spv_magic);
	return TEST_REPORT();
}
