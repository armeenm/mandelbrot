#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include <VkBootstrap.h>

#include "util.h"
#include "conf.h"

auto main() -> int {
  spdlog::info("GLFW Vulkan support? {}\n", glfwVulkanSupported());
}
