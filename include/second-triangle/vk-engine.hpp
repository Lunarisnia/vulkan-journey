#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
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
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  VkSwapchainKHR swapchain;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

 public:
  void Init();
  void Cleanup();

 private:
  void initWindow();
  void initVulkan();
  void initSwapchain();
  void initCommand();
  void initSyncStructures();

 private:
  void createSwapchain(uint32_t width, uint32_t height);
  void destroySwapchain();
};
