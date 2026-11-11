#include "vulkan_app.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <span>

namespace {

#ifdef DEBUG
constexpr bool g_enable_validation_layers = true;
#else
constexpr bool g_enable_validation_layers = false;
#endif

constexpr std::array<const char *, 1> g_validation_layers = {
		"VK_LAYER_KHRONOS_validation"};

constexpr std::array<const char *, 1> g_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT /*message_severity*/,
		VkDebugUtilsMessageTypeFlagsEXT /*message_type*/,
		const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
		void * /*user_data*/) {
	std::cerr << std::format("[Vulkan Validation] {}\n", callback_data->pMessage);
	return VK_FALSE;
}

VkResult create_debug_utils_messenger_ext(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *create_info,
		const VkAllocationCallbacks *allocator,
		VkDebugUtilsMessengerEXT *debug_messenger) {
	auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if(func != nullptr) {
		return func(instance, create_info, allocator, debug_messenger);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_utils_messenger_ext(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debug_messenger,
		const VkAllocationCallbacks *allocator) {
	auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	if(func != nullptr) {
		func(instance, debug_messenger, allocator);
	}
}

bool check_validation_layer_support() {
	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	if(layer_count == 0) {
		return false;
	}

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for(const char *layer_name : g_validation_layers) {
		bool layer_found = false;
		for(const auto &layer_props : available_layers) {
			if(std::strcmp(layer_name, layer_props.layerName) == 0) {
				layer_found = true;
				break;
			}
		}
		if(!layer_found) {
			return false;
		}
	}
	return true;
}

std::optional<std::vector<char>> read_binary_file(std::string_view filename) {
	std::array<std::string, 4> candidates = {
			std::string(filename),
			std::string("shaders/") + std::string(filename),
			std::string("build/debug/bin/shaders/") + std::string(filename),
			std::string("build/release/bin/shaders/") + std::string(filename)};

	for(const auto &path : candidates) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if(file.is_open()) {
			const auto file_size = file.tellg();
			if(file_size <= 0) {
				return std::nullopt;
			}

			std::vector<char> buffer(static_cast<size_t>(file_size));
			file.seekg(0);
			file.read(buffer.data(), file_size);
			return buffer;
		}
	}

	std::cerr << std::format("error: Could not open shader file '{}'\n", filename);
	return std::nullopt;
}

VkShaderModule create_shader_module(VkDevice device, std::span<const char> code) {
	VkShaderModuleCreateInfo create_info {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = code.size(),
			.pCode = reinterpret_cast<const uint32_t *>(code.data())};

	VkShaderModule shader_module = VK_NULL_HANDLE;
	if(vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		return VK_NULL_HANDLE;
	}
	return shader_module;
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	[[nodiscard]] bool is_complete() const noexcept {
		return graphics_family.has_value() && present_family.has_value();
	}
};

QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	if(queue_family_count == 0) {
		return indices;
	}

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	for(uint32_t i = 0; i < queue_family_count; ++i) {
		if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		VkBool32 present_support = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
		if(present_support) {
			indices.present_family = i;
		}

		if(indices.is_complete()) {
			break;
		}
	}
	return indices;
}

bool check_device_extension_support(VkPhysicalDevice device) {
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
	if(extension_count == 0) {
		return false;
	}

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

	for(const char *extension_name : g_device_extensions) {
		bool extension_found = false;
		for(const auto &ext : available_extensions) {
			if(std::strcmp(extension_name, ext.extensionName) == 0) {
				extension_found = true;
				break;
			}
		}
		if(!extension_found) {
			return false;
		}
	}
	return true;
}

int rate_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface) {
	const auto indices = find_queue_families(device, surface);
	if(!indices.is_complete()) {
		return 0;
	}

	if(!check_device_extension_support(device)) {
		return 0;
	}

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

	if(format_count == 0 || present_mode_count == 0) {
		return 0;
	}

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	int score = 100;
	if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	} else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
		score += 500;
	}
	return score;
}

VkSurfaceFormatKHR choose_swap_surface_format(std::span<const VkSurfaceFormatKHR> available_formats) {
	for(const auto &format : available_formats) {
		if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
		   format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}
	return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(std::span<const VkPresentModeKHR> available_modes) {
	for(const auto &mode : available_modes) {
		if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return mode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed available
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actual_extent {
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height)};

	actual_extent.width = std::clamp(
			actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actual_extent.height = std::clamp(
			actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actual_extent;
}

} // namespace

VulkanApp::~VulkanApp() {
	cleanup();
}

void VulkanApp::framebuffer_resize_callback(GLFWwindow *window, int /*width*/, int /*height*/) {
	auto *app = static_cast<VulkanApp *>(glfwGetWindowUserPointer(window));
	if(app) {
		app->m_framebuffer_resized = true;
	}
}

bool VulkanApp::init_window() {
	if(!glfwInit()) {
		std::cerr << "error: Failed to initialize GLFW\n";
		return false;
	}

	if(!glfwVulkanSupported()) {
		std::cerr << "error: Vulkan is not supported by the local graphics driver\n";
		glfwTerminate();
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	const std::string title_str(m_title);
	m_window = glfwCreateWindow(
			static_cast<int>(m_width), static_cast<int>(m_height), title_str.c_str(), nullptr, nullptr);
	if(!m_window) {
		std::cerr << "error: Failed to create GLFW window\n";
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, framebuffer_resize_callback);
	return true;
}

bool VulkanApp::create_instance() {
	bool use_validation = g_enable_validation_layers;
	if(use_validation && !check_validation_layer_support()) {
		std::cerr << "warning: Validation layers requested, but not available. Continuing without.\n";
		use_validation = false;
	}

	const std::string app_name(m_title.empty() ? "Vulkan Triangle" : m_title);
	VkApplicationInfo app_info {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = app_name.c_str(),
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_0};

	uint32_t glfw_extension_count = 0;
	const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	if(!glfw_extensions) {
		std::cerr << "error: Failed to retrieve required GLFW instance extensions\n";
		return false;
	}

	std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
	if(use_validation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// macOS MoltenVK portability enumeration check
	uint32_t available_extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
	bool has_portability_enumeration = false;
	if(available_extension_count > 0) {
		std::vector<VkExtensionProperties> available_exts(available_extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_exts.data());
		for(const auto &ext : available_exts) {
			if(std::strcmp(ext.extensionName, "VK_KHR_portability_enumeration") == 0) {
				has_portability_enumeration = true;
				break;
			}
		}
	}

	VkInstanceCreateInfo create_info {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &app_info,
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data()};

	if(has_portability_enumeration) {
		extensions.push_back("VK_KHR_portability_enumeration");
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();
		create_info.flags |= 0x00000001; // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
	}

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
	if(use_validation) {
		create_info.enabledLayerCount = static_cast<uint32_t>(g_validation_layers.size());
		create_info.ppEnabledLayerNames = g_validation_layers.data();

		debug_create_info = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = debug_callback};
		create_info.pNext = &debug_create_info;
	}

	if(vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
		std::cerr << "error: vkCreateInstance failed\n";
		return false;
	}

	if(use_validation) {
		create_debug_utils_messenger_ext(m_instance, &debug_create_info, nullptr, &m_debug_messenger);
	}
	return true;
}

bool VulkanApp::create_surface() {
	if(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
		std::cerr << "error: glfwCreateWindowSurface failed\n";
		return false;
	}
	return true;
}

bool VulkanApp::pick_physical_device() {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
	if(device_count == 0) {
		std::cerr << "error: Failed to find GPUs with Vulkan support\n";
		return false;
	}

	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

	VkPhysicalDevice best_device = VK_NULL_HANDLE;
	int best_score = 0;

	for(const auto &device : devices) {
		int score = rate_device_suitability(device, m_surface);
		if(score > best_score) {
			best_device = device;
			best_score = score;
		}
	}

	if(best_device == VK_NULL_HANDLE) {
		std::cerr << "error: Failed to find a suitable GPU for Vulkan rendering\n";
		return false;
	}

	m_physical_device = best_device;
	const auto indices = find_queue_families(best_device, m_surface);
	m_graphics_queue_family = indices.graphics_family.value();
	m_present_queue_family = indices.present_family.value();

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(best_device, &props);
	std::cout << std::format("Selected Vulkan GPU: {}\n", props.deviceName);

	return true;
}

bool VulkanApp::create_logical_device() {
	float queue_priority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	queue_create_infos.push_back({.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
								  .queueFamilyIndex = m_graphics_queue_family,
								  .queueCount = 1,
								  .pQueuePriorities = &queue_priority});

	if(m_graphics_queue_family != m_present_queue_family) {
		queue_create_infos.push_back({.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
									  .queueFamilyIndex = m_present_queue_family,
									  .queueCount = 1,
									  .pQueuePriorities = &queue_priority});
	}

	VkPhysicalDeviceFeatures device_features {};

	std::vector<const char *> enabled_extensions(g_device_extensions.begin(), g_device_extensions.end());

	// Check for MoltenVK portability subset
	uint32_t ext_count = 0;
	vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &ext_count, nullptr);
	if(ext_count > 0) {
		std::vector<VkExtensionProperties> ext_props(ext_count);
		vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &ext_count, ext_props.data());
		for(const auto &ext : ext_props) {
			if(std::strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
				enabled_extensions.push_back("VK_KHR_portability_subset");
				break;
			}
		}
	}

	VkDeviceCreateInfo create_info {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
			.pQueueCreateInfos = queue_create_infos.data(),
			.enabledLayerCount = g_enable_validation_layers ? static_cast<uint32_t>(g_validation_layers.size()) : 0,
			.ppEnabledLayerNames = g_enable_validation_layers ? g_validation_layers.data() : nullptr,
			.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
			.ppEnabledExtensionNames = enabled_extensions.data(),
			.pEnabledFeatures = &device_features};

	if(vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS) {
		std::cerr << "error: vkCreateDevice failed\n";
		return false;
	}

	vkGetDeviceQueue(m_device, m_graphics_queue_family, 0, &m_graphics_queue);
	vkGetDeviceQueue(m_device, m_present_queue_family, 0, &m_present_queue);
	return true;
}

bool VulkanApp::create_swapchain() {
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &capabilities);

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, formats.data());

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &present_mode_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &present_mode_count, present_modes.data());

	const auto surface_format = choose_swap_surface_format(formats);
	const auto present_mode = choose_swap_present_mode(present_modes);
	const auto extent = choose_swap_extent(capabilities, m_window);

	uint32_t image_count = capabilities.minImageCount + 1;
	if(capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_surface,
			.minImageCount = image_count,
			.imageFormat = surface_format.format,
			.imageColorSpace = surface_format.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = present_mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE};

	std::array<uint32_t, 2> queue_family_indices = {m_graphics_queue_family, m_present_queue_family};
	if(m_graphics_queue_family != m_present_queue_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices.data();
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	if(vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS) {
		std::cerr << "error: vkCreateSwapchainKHR failed\n";
		return false;
	}

	uint32_t actual_image_count = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &actual_image_count, nullptr);
	m_swapchain_images.resize(actual_image_count);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &actual_image_count, m_swapchain_images.data());

	m_swapchain_image_format = surface_format.format;
	m_swapchain_extent = extent;
	return true;
}

bool VulkanApp::create_image_views() {
	m_swapchain_image_views.resize(m_swapchain_images.size());

	for(size_t i = 0; i < m_swapchain_images.size(); ++i) {
		VkImageViewCreateInfo create_info {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = m_swapchain_images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = m_swapchain_image_format,
				.components = {
						.r = VK_COMPONENT_SWIZZLE_IDENTITY,
						.g = VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = VK_COMPONENT_SWIZZLE_IDENTITY,
						.a = VK_COMPONENT_SWIZZLE_IDENTITY},
				.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};

		if(vkCreateImageView(m_device, &create_info, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS) {
			std::cerr << std::format("error: vkCreateImageView failed for image {}\n", i);
			return false;
		}
	}
	return true;
}

bool VulkanApp::create_render_pass() {
	VkAttachmentDescription color_attachment {
			.format = m_swapchain_image_format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

	VkAttachmentReference color_attachment_ref {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

	VkSubpassDescription subpass {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment_ref};

	VkSubpassDependency dependency {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

	VkRenderPassCreateInfo render_pass_info {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &color_attachment,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency};

	if(vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS) {
		std::cerr << "error: vkCreateRenderPass failed\n";
		return false;
	}
	return true;
}

bool VulkanApp::create_graphics_pipeline() {
	const auto vert_code = read_binary_file("triangle.vert.spv");
	const auto frag_code = read_binary_file("triangle.frag.spv");

	if(!vert_code || !frag_code) {
		return false;
	}

	VkShaderModule vert_module = create_shader_module(m_device, *vert_code);
	VkShaderModule frag_module = create_shader_module(m_device, *frag_code);

	if(vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
		if(vert_module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(m_device, vert_module, nullptr);
		}
		if(frag_module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(m_device, frag_module, nullptr);
		}
		return false;
	}

	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {{{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
																	  .stage = VK_SHADER_STAGE_VERTEX_BIT,
																	  .module = vert_module,
																	  .pName = "main"},
																	 {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
																	  .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
																	  .module = frag_module,
																	  .pName = "main"}}};

	constexpr std::array<VkDynamicState, 2> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamic_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
			.pDynamicStates = dynamic_states.data()};

	VkPipelineVertexInputStateCreateInfo vertex_input_info {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	VkPipelineInputAssemblyStateCreateInfo input_assembly {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE};

	VkPipelineViewportStateCreateInfo viewport_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1};

	VkPipelineRasterizationStateCreateInfo rasterizer {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.lineWidth = 1.0f};

	VkPipelineMultisampleStateCreateInfo multisampling {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE};

	VkPipelineColorBlendAttachmentState color_blend_attachment {
			.blendEnable = VK_FALSE,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
							  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

	VkPipelineColorBlendStateCreateInfo color_blending {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.attachmentCount = 1,
			.pAttachments = &color_blend_attachment};

	VkPushConstantRange push_constant_range {
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = 0,
			.size = sizeof(float)};

	VkPipelineLayoutCreateInfo pipeline_layout_info {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &push_constant_range};

	if(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
		std::cerr << "error: vkCreatePipelineLayout failed\n";
		vkDestroyShaderModule(m_device, vert_module, nullptr);
		vkDestroyShaderModule(m_device, frag_module, nullptr);
		return false;
	}

	VkGraphicsPipelineCreateInfo pipeline_info {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = static_cast<uint32_t>(shader_stages.size()),
			.pStages = shader_stages.data(),
			.pVertexInputState = &vertex_input_info,
			.pInputAssemblyState = &input_assembly,
			.pViewportState = &viewport_state,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &color_blending,
			.pDynamicState = &dynamic_state,
			.layout = m_pipeline_layout,
			.renderPass = m_render_pass,
			.subpass = 0};

	VkResult result = vkCreateGraphicsPipelines(
			m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline);

	vkDestroyShaderModule(m_device, vert_module, nullptr);
	vkDestroyShaderModule(m_device, frag_module, nullptr);

	if(result != VK_SUCCESS) {
		std::cerr << "error: vkCreateGraphicsPipelines failed\n";
		return false;
	}
	return true;
}

bool VulkanApp::create_framebuffers() {
	m_swapchain_framebuffers.resize(m_swapchain_image_views.size());

	for(size_t i = 0; i < m_swapchain_image_views.size(); ++i) {
		std::array<VkImageView, 1> attachments = {m_swapchain_image_views[i]};

		VkFramebufferCreateInfo framebuffer_info {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = m_render_pass,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = m_swapchain_extent.width,
				.height = m_swapchain_extent.height,
				.layers = 1};

		if(vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &m_swapchain_framebuffers[i]) != VK_SUCCESS) {
			std::cerr << std::format("error: vkCreateFramebuffer failed for buffer {}\n", i);
			return false;
		}
	}
	return true;
}

bool VulkanApp::create_command_pool() {
	VkCommandPoolCreateInfo pool_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = m_graphics_queue_family};

	if(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS) {
		std::cerr << "error: vkCreateCommandPool failed\n";
		return false;
	}
	return true;
}

bool VulkanApp::create_command_buffers() {
	VkCommandBufferAllocateInfo alloc_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = MAX_FRAMES_IN_FLIGHT};

	if(vkAllocateCommandBuffers(m_device, &alloc_info, m_command_buffers.data()) != VK_SUCCESS) {
		std::cerr << "error: vkAllocateCommandBuffers failed\n";
		return false;
	}
	return true;
}

void VulkanApp::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
	VkCommandBufferBeginInfo begin_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkClearValue clear_color {{{0.012f, 0.012f, 0.024f, 1.0f}}};

	VkRenderPassBeginInfo render_pass_info {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = m_render_pass,
			.framebuffer = m_swapchain_framebuffers[image_index],
			.renderArea = {
					.offset = {0, 0},
					.extent = m_swapchain_extent},
			.clearValueCount = 1,
			.pClearValues = &clear_color};

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

	VkViewport viewport {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(m_swapchain_extent.width),
			.height = static_cast<float>(m_swapchain_extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor {
			.offset = {0, 0},
			.extent = m_swapchain_extent};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	// Push elapsed time to the fragment shader for dynamic color updates
	float current_time = static_cast<float>(glfwGetTime());
	vkCmdPushConstants(
			command_buffer,
			m_pipeline_layout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(float),
			&current_time);

	// Draw 3 bufferless vertices defined directly in triangle.vert
	vkCmdDraw(command_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(command_buffer);
	vkEndCommandBuffer(command_buffer);
}

bool VulkanApp::create_sync_objects() {
	VkSemaphoreCreateInfo semaphore_info {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

	VkFenceCreateInfo fence_info {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT};

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS ||
		   vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS ||
		   vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS) {
			std::cerr << std::format("error: Failed to create sync objects for frame {}\n", i);
			return false;
		}
	}
	return true;
}

void VulkanApp::cleanup_swapchain() {
	for(auto framebuffer : m_swapchain_framebuffers) {
		if(framebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}
	}
	m_swapchain_framebuffers.clear();

	for(auto image_view : m_swapchain_image_views) {
		if(image_view != VK_NULL_HANDLE) {
			vkDestroyImageView(m_device, image_view, nullptr);
		}
	}
	m_swapchain_image_views.clear();

	if(m_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
		m_swapchain = VK_NULL_HANDLE;
	}
}

void VulkanApp::recreate_swapchain() {
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while(width == 0 || height == 0) {
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_device);

	cleanup_swapchain();

	create_swapchain();
	create_image_views();
	create_framebuffers();
}

void VulkanApp::draw_frame() {
	vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index = 0;
	VkResult result = vkAcquireNextImageKHR(
			m_device,
			m_swapchain,
			UINT64_MAX,
			m_image_available_semaphores[m_current_frame],
			VK_NULL_HANDLE,
			&image_index);

	if(result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain();
		return;
	}
	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::cerr << std::format("error: vkAcquireNextImageKHR failed (code {})\n", static_cast<int>(result));
		return;
	}

	vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

	vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
	record_command_buffer(m_command_buffers[m_current_frame], image_index);

	VkSemaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame]};

	VkSubmitInfo submit_info {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = wait_semaphores,
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_command_buffers[m_current_frame],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signal_semaphores};

	if(vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
		std::cerr << "error: vkQueueSubmit failed\n";
		return;
	}

	VkSwapchainKHR swapchains[] = {m_swapchain};
	VkPresentInfoKHR present_info {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signal_semaphores,
			.swapchainCount = 1,
			.pSwapchains = swapchains,
			.pImageIndices = &image_index};

	result = vkQueuePresentKHR(m_present_queue, &present_info);

	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized) {
		m_framebuffer_resized = false;
		recreate_swapchain();
	} else if(result != VK_SUCCESS) {
		std::cerr << std::format("error: vkQueuePresentKHR failed (code {})\n", static_cast<int>(result));
	}

	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool VulkanApp::init(uint32_t width, uint32_t height, std::string_view title) {
	m_width = width;
	m_height = height;
	m_title = title;

	return init_window() &&
		   create_instance() &&
		   create_surface() &&
		   pick_physical_device() &&
		   create_logical_device() &&
		   create_swapchain() &&
		   create_image_views() &&
		   create_render_pass() &&
		   create_graphics_pipeline() &&
		   create_framebuffers() &&
		   create_command_pool() &&
		   create_command_buffers() &&
		   create_sync_objects();
}

void VulkanApp::run() {
	while(!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		draw_frame();
	}

	if(m_device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_device);
	}
}

void VulkanApp::cleanup() {
	if(m_device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_device);

		for(auto semaphore : m_render_finished_semaphores) {
			if(semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(m_device, semaphore, nullptr);
			}
		}
		m_render_finished_semaphores.fill(VK_NULL_HANDLE);

		for(auto semaphore : m_image_available_semaphores) {
			if(semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(m_device, semaphore, nullptr);
			}
		}
		m_image_available_semaphores.fill(VK_NULL_HANDLE);

		for(auto fence : m_in_flight_fences) {
			if(fence != VK_NULL_HANDLE) {
				vkDestroyFence(m_device, fence, nullptr);
			}
		}
		m_in_flight_fences.fill(VK_NULL_HANDLE);

		if(m_command_pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(m_device, m_command_pool, nullptr);
			m_command_pool = VK_NULL_HANDLE;
		}

		cleanup_swapchain();

		if(m_graphics_pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
			m_graphics_pipeline = VK_NULL_HANDLE;
		}

		if(m_pipeline_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
			m_pipeline_layout = VK_NULL_HANDLE;
		}

		if(m_render_pass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(m_device, m_render_pass, nullptr);
			m_render_pass = VK_NULL_HANDLE;
		}

		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}

	if(m_debug_messenger != VK_NULL_HANDLE) {
		destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, nullptr);
		m_debug_messenger = VK_NULL_HANDLE;
	}

	if(m_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}

	if(m_instance != VK_NULL_HANDLE) {
		vkDestroyInstance(m_instance, nullptr);
		m_instance = VK_NULL_HANDLE;
	}

	if(m_window) {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}

	glfwTerminate();
}
