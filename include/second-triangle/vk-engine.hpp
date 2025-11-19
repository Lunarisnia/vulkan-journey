#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include "SDL_video.h"
struct FrameData {
  VkCommandPool commandPool;
  VkCommandBuffer mainCommandBuffer;

  VkSemaphore swapchainSemaphore;
  VkSemaphore renderSemaphore;
  VkFence renderFence;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
 public:
  VkExtent2D windowExtent{1700, 900};
  bool bUseValidationLayer = true;

 private:
  bool stopRendering;
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
  unsigned int frameNumber;

  FrameData frames[FRAME_OVERLAP];

  VkQueue graphicsQueue;
  uint32_t graphicsQueueFamily;

 public:
  void Init();
  void Run();
  void Cleanup();
  FrameData& GetCurrentFrame();
  void Draw();

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
