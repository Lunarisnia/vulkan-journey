#include "second-triangle/vk-engine.hpp"
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_vulkan.h"
#include "VkBootstrap.h"
#include "vulkan/vulkan_core.h"
void VulkanEngine::Init() {
  initWindow();
  initVulkan();
  initSwapchain();
  initCommand();
  initSyncStructures();

  initialized = true;
}

void VulkanEngine::Cleanup() {
  if (initialized) {
    vkDeviceWaitIdle(device);

    for (FrameData& frame : frames) {
      vkDestroyCommandPool(device, frame.commandPool, nullptr);
    }

    // Destroy swapchain
    destroySwapchain();

    // Destroy surface
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // Destroy device
    vkDestroyDevice(device, nullptr);

    // Destroy extensions
    vkb::destroy_debug_utils_messenger(instance, debugMessenger, nullptr);

    // Destroy instance
    vkDestroyInstance(instance, nullptr);
    // Destroy window
    SDL_DestroyWindow(window);
  }
}

FrameData& VulkanEngine::GetCurrentFrame() {
  return frames[frameNumber % FRAME_OVERLAP];
}

void VulkanEngine::initWindow() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  window = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, windowExtent.width,
                            windowExtent.height, windowFlags);
}

void VulkanEngine::initVulkan() {
  vkb::InstanceBuilder builder;

  auto instanceResult = builder.set_app_name("Example Vulkan App")
                            .request_validation_layers(bUseValidationLayer)
                            .use_default_debug_messenger()
                            .require_api_version(1, 3, 0)
                            .build();
  vkb::Instance vkbInstance = instanceResult.value();

  instance = vkbInstance.instance;
  debugMessenger = vkbInstance.debug_messenger;

  SDL_Vulkan_CreateSurface(window, instance, &surface);

  // vulkan 1.3 features
  VkPhysicalDeviceVulkan13Features features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  features.dynamicRendering = true;
  features.synchronization2 = true;

  // vulkan 1.2 features
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  // use vkbootstrap to select a gpu.
  // We want a gpu that can write to the SDL surface and supports vulkan 1.3
  // with the correct features
  // NOTE: I think what this does is that it loop over all the GPU and check if
  // it fulfill all the required features. (Need to check more into this)
  vkb::PhysicalDeviceSelector selector{vkbInstance};
  vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                           .set_required_features_13(features)
                                           .set_required_features_12(features12)
                                           .set_surface(surface)
                                           .select()
                                           .value();

  vkb::DeviceBuilder deviceBuilder{physicalDevice};
  vkb::Device vkbDevice = deviceBuilder.build().value();

  device = vkbDevice.device;
  chosenGPU = physicalDevice.physical_device;

  // use vkbootstrap to get a Graphics queue
  graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::createSwapchain(uint32_t width, uint32_t height) {
  vkb::SwapchainBuilder swapchainBuilder{chosenGPU, device, surface};

  swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          .set_desired_format(
              VkSurfaceFormatKHR{.format = swapchainImageFormat})
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(width, height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  swapchainExtent = vkbSwapchain.extent;
  swapchain = vkbSwapchain.swapchain;

  swapchainImages = vkbSwapchain.get_images().value();
  swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanEngine::destroySwapchain() {
  vkDestroySwapchainKHR(device, swapchain, nullptr);

  // destroy swapchain resources
  for (int i = 0; i < swapchainImageViews.size(); i++) {
    vkDestroyImageView(device, swapchainImageViews[i], nullptr);
  }
}

void VulkanEngine::initSwapchain() {
  createSwapchain(windowExtent.width, windowExtent.height);
}

void VulkanEngine::initCommand() {
  VkCommandPoolCreateInfo commandPoolInfo = {};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.pNext = nullptr;
  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolInfo.queueFamilyIndex = graphicsQueueFamily;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkCreateCommandPool(device, &commandPoolInfo, nullptr,
                        &frames[i].commandPool);
    // allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = frames[i].commandPool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(device, &cmdAllocInfo,
                             &frames[i].mainCommandBuffer);
  }
}

void VulkanEngine::initSyncStructures() {}
