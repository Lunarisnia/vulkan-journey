#pragma once
#include <vulkan/vulkan_core.h>
#include "SDL_video.h"
class VulkanEngine {
 public:
  VkExtent2D windowExtent{1700, 900};
  bool bUseValidationLayer = true;

 private:
  SDL_Window* window;
  bool initialized;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;
  VkDevice device;
  VkPhysicalDevice chosenGPU;

 public:
  void Init();

 private:
  void initWindow();
  void initVulkan();
  void initSwapchain();
  void initCommand();
  void initSyncStructures();
};
