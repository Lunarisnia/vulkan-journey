#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "SDL_video.h"
#include "second-triangle/deletion-queue.hpp"
#include "second-triangle/vk-descriptors.hpp"
struct FrameData {
  VkCommandPool commandPool;
  VkCommandBuffer mainCommandBuffer;

  VkSemaphore swapchainSemaphore;
  VkFence renderFence;

  DeletionQueue deletionQueue;
};

struct AllocatedImage {
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;
  VkExtent3D imageExtent;
  VkFormat imageFormat;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
 public:
  VkExtent2D windowExtent{800, 600};
  bool bUseValidationLayer = true;
  VkPipeline gradientPipeline;
  VkPipelineLayout gradientPipelineLayout;

 private:
  AllocatedImage drawImage;
  VkExtent2D drawExtent;
  VmaAllocator allocator;

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
  std::vector<VkSemaphore> renderSemaphores;
  unsigned int frameNumber = 0;

  FrameData frames[FRAME_OVERLAP];

  VkQueue graphicsQueue;
  uint32_t graphicsQueueFamily;

  DeletionQueue deletionQueue;

  DescriptorAllocator globalDescriptorAllocator;
  VkDescriptorSet drawImageDescriptors;
  VkDescriptorSetLayout drawImageDescriptorLayout;

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
  void initDescriptors();
  void initPipelines();
  void initBackgroundPipelines();

 private:
  void createSwapchain(uint32_t width, uint32_t height);
  void destroySwapchain();
  void drawBackground(VkCommandBuffer cmd);
};
