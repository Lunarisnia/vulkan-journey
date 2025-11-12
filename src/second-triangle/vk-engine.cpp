#include "second-triangle/vk-engine.hpp"
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_vulkan.h"
#include "VkBootstrap.h"
#include "vulkan/vulkan.hpp"
void VulkanEngine::Init() {
  initWindow();
  initVulkan();
  initSwapchain();
  initCommand();
  initSyncStructures();

  initialized = true;
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
}

void VulkanEngine::initSwapchain() {}

void VulkanEngine::initCommand() {}

void VulkanEngine::initSyncStructures() {}
