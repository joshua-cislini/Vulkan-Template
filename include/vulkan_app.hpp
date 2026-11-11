/**
 * @file vulkan_app.hpp
 * @brief Starter Vulkan application class and rendering coordinator (C++23 Template).
 */
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/**
 * @brief Complete Vulkan application lifecycle coordinator using Pragmatic C++23.
 *
 * Implements deterministic RAII resource management for all Vulkan and GLFW handles.
 */
class VulkanApp {
  public:
	/** Maximum number of frames being processed concurrently by the CPU/GPU. */
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	VulkanApp() = default;
	~VulkanApp();

	// Non-copyable, non-movable to guarantee single handle ownership
	VulkanApp(const VulkanApp &) = delete;
	VulkanApp &operator=(const VulkanApp &) = delete;
	VulkanApp(VulkanApp &&) = delete;
	VulkanApp &operator=(VulkanApp &&) = delete;

	/**
	 * @brief Initializes GLFW, the window, Vulkan instance, device, swapchain, and pipeline.
	 *
	 * @param width Window width in screen coordinates.
	 * @param height Window height in screen coordinates.
	 * @param title Window title string.
	 * @return true if initialization succeeded, false otherwise.
	 */
	bool init(uint32_t width, uint32_t height, std::string_view title);

	/**
	 * @brief Runs the main application rendering loop until the window is closed.
	 */
	void run();

	/**
	 * @brief Explicitly releases all Vulkan and GLFW resources. Also invoked by destructor.
	 */
	void cleanup();

  private:
	// Window & dimensions
	GLFWwindow *m_window = nullptr;
	uint32_t m_width = 800;
	uint32_t m_height = 600;
	std::string_view m_title;
	bool m_framebuffer_resized = false;

	// Vulkan instance & debug
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	// Devices & queues
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	uint32_t m_graphics_queue_family = 0;
	uint32_t m_present_queue_family = 0;
	VkQueue m_graphics_queue = VK_NULL_HANDLE;
	VkQueue m_present_queue = VK_NULL_HANDLE;

	// Swapchain
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkFormat m_swapchain_image_format = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swapchain_extent {};
	std::vector<VkImage> m_swapchain_images;
	std::vector<VkImageView> m_swapchain_image_views;
	std::vector<VkFramebuffer> m_swapchain_framebuffers;

	// Pipeline
	VkRenderPass m_render_pass = VK_NULL_HANDLE;
	VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
	VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

	// Commands
	VkCommandPool m_command_pool = VK_NULL_HANDLE;
	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_command_buffers {};

	// Synchronization
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_image_available_semaphores {};
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_render_finished_semaphores {};
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_in_flight_fences {};

	uint32_t m_current_frame = 0;

	// Internal initialization helpers
	bool init_window();
	bool create_instance();
	bool create_surface();
	bool pick_physical_device();
	bool create_logical_device();
	bool create_swapchain();
	bool create_image_views();
	bool create_render_pass();
	bool create_graphics_pipeline();
	bool create_framebuffers();
	bool create_command_pool();
	bool create_command_buffers();
	bool create_sync_objects();

	// Frame rendering and swapchain recreation
	void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
	void draw_frame();
	void recreate_swapchain();
	void cleanup_swapchain();

	static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);
};
