#include "second-triangle/vk-engine.hpp"
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_vulkan.h"
#include "VkBootstrap.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "fmt/base.h"
#include "imgui.h"
#include "second-triangle/vk-image.hpp"
#include "second-triangle/vk-initializer.hpp"
#include "second-triangle/vk-pipelines.hpp"
#include "vulkan/vulkan_core.h"
#include <cmath>
#include <cstdint>
#include <thread>
void VulkanEngine::Init() {
  initWindow();
  initVulkan();
  initSwapchain();
  initCommand();
  initSyncStructures();
  initDescriptors();
  initPipelines();
  initImgui();

  initialized = true;
}

void VulkanEngine::Run() {
  SDL_Event e;
  bool bQuit = false;

  // main loop
  while (!bQuit) {
    // Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      // close the window when user alt-f4s or clicks the X button
      if (e.type == SDL_QUIT)
        bQuit = true;

      if (e.type == SDL_WINDOWEVENT) {

        if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
          stopRendering = true;
        }
        if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
          stopRendering = false;
        }
      }

      // send SDL event to imgui for handling
      ImGui_ImplSDL2_ProcessEvent(&e);

      // close the window when user alt-f4s or clicks the X button
      // if (e.type == SDL_QUIT)
      //   bQuit = true;
      //
      // if (e.type == SDL_WINDOWEVENT) {
      //   if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
      //     stopRendering = true;
      //   }
      //   if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
      //     stopRendering = false;
      //   }
      // }
    }

    // do not draw if we are minimized
    if (stopRendering) {
      // throttle the speed to avoid the endless spinning
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      // some imgui UI to test
      ImGui::ShowDemoWindow();

      // make imgui calculate internal draw structures
      ImGui::Render();

      Draw();
    }
  }
}

void VulkanEngine::Cleanup() {
  if (initialized) {
    vkDeviceWaitIdle(device);

    for (FrameData &frame : frames) {
      vkDestroyCommandPool(device, frame.commandPool, nullptr);

      vkDestroyFence(device, frame.renderFence, nullptr);
      vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);

      frame.deletionQueue.Flush();
    }

    deletionQueue.Flush();

    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
      vkDestroySemaphore(device, renderSemaphores[i], nullptr);
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

void VulkanEngine::Draw() {
  FrameData currentFrame = GetCurrentFrame();

  vkWaitForFences(device, 1, &currentFrame.renderFence, true, 1000000000);
  vkResetFences(device, 1, &currentFrame.renderFence);

  currentFrame.deletionQueue.Flush();

  uint32_t swapchainImageIndex;
  vkAcquireNextImageKHR(device, swapchain, 1000000000,
                        currentFrame.swapchainSemaphore, nullptr,
                        &swapchainImageIndex);

  VkCommandBuffer cmd = currentFrame.mainCommandBuffer;

  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::CommandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  drawExtent.width = drawImage.imageExtent.width;
  drawExtent.height = drawImage.imageExtent.height;

  // This render to a render target then copy the image to the presentable
  // swapchain
  VulkanImage::TransitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_GENERAL);
  drawBackground(cmd);
  // make the swapchain image into presentable mode
  VulkanImage::TransitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  VulkanImage::TransitionImage(cmd, swapchainImages[swapchainImageIndex],
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Copy the image to the swapchain
  VulkanImage::BlitImage(cmd, drawImage.image,
                         swapchainImages[swapchainImageIndex], drawExtent,
                         swapchainExtent);

  // set swapchain image layout to Attachment Optimal so we can draw it
  VulkanImage::TransitionImage(cmd, swapchainImages[swapchainImageIndex],
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  // draw imgui into the swapchain image
  drawImgui(cmd, swapchainImageViews[swapchainImageIndex]);

  VulkanImage::TransitionImage(cmd, swapchainImages[swapchainImageIndex],
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // finalize the command buffer (we can no longer add commands, but it can
  // now be executed)
  vkEndCommandBuffer(cmd);

  // prepare the submission to the queue.
  // we want to wait on the _swapchainSemaphore, as that semaphore is signaled
  // when the swapchain is ready we will signal the _renderSemaphore, to signal
  // that rendering has finished

  // NOTE: use render semaphore that is guaranteed to be available by indexing
  // it with the swapchainImageIndex
  VkSemaphore renderSemaphore = renderSemaphores[swapchainImageIndex];

  VkCommandBufferSubmitInfo cmdinfo = VulkanInit::CommandBufferSubmitInfo(cmd);

  VkSemaphoreSubmitInfo waitInfo = VulkanInit::SemaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      currentFrame.swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo = VulkanInit::SemaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, renderSemaphore);

  VkSubmitInfo2 submit =
      VulkanInit::SubmitInfo(&cmdinfo, &signalInfo, &waitInfo);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  vkQueueSubmit2(graphicsQueue, 1, &submit, currentFrame.renderFence);

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  vkQueuePresentKHR(graphicsQueue, &presentInfo);

  // increase the number of frames drawn
  frameNumber++;
}

void VulkanEngine::drawBackground(VkCommandBuffer cmd) {
  // make a clear-color from frame number. This will flash with a 120 frame
  // period.
  // VkClearColorValue clearValue;
  // float flash = std::abs(std::sin(frameNumber / 120.f));
  // clearValue = {{0.0f, 0.0f, flash, 1.0f}};
  //
  // VkImageSubresourceRange clearRange =
  //     VulkanInit::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
  //
  // // clear image
  // vkCmdClearColorImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
  //                      &clearValue, 1, &clearRange);

  // bind the gradient drawing compute pipeline
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradientPipeline);

  // bind the descriptor set containing the draw image for the compute pipeline
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          gradientPipelineLayout, 0, 1, &drawImageDescriptors,
                          0, nullptr);

  // execute the compute pipeline dispatch. We are using 16x16 workgroup size so
  // we need to divide by it
  vkCmdDispatch(cmd, std::ceil(drawExtent.width / 16.0),
                std::ceil(drawExtent.height / 16.0), 1);
}

FrameData &VulkanEngine::GetCurrentFrame() {
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

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = chosenGPU;
  allocatorInfo.device = device;
  allocatorInfo.instance = instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &allocator);

  deletionQueue.PushFunction([&]() { vmaDestroyAllocator(allocator); });
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

  VkExtent3D drawImageExtent = {windowExtent.width, windowExtent.height, 1};

  // hardcoding the draw format to 32 bit float
  drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  drawImage.imageExtent = drawImageExtent;

  VkImageUsageFlags drawImageUsages{};
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo rImgInfo = VulkanInit::ImageCreateInfo(
      drawImage.imageFormat, drawImageUsages, drawImageExtent);

  // for the draw image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo rImgAllocInfo = {};
  rImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  rImgAllocInfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  vmaCreateImage(allocator, &rImgInfo, &rImgAllocInfo, &drawImage.image,
                 &drawImage.allocation, nullptr);
  // build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo rViewInfo = VulkanInit::ImageViewCreateInfo(
      drawImage.imageFormat, drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

  vkCreateImageView(device, &rViewInfo, nullptr, &drawImage.imageView);

  // add to deletion queues
  deletionQueue.PushFunction([&]() {
    vkDestroyImageView(device, drawImage.imageView, nullptr);
    vmaDestroyImage(allocator, drawImage.image, drawImage.allocation);
  });
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

  vkCreateCommandPool(device, &commandPoolInfo, nullptr, &immCommandPool);

  VkCommandBufferAllocateInfo immCommandAllocateInfo = {};
  immCommandAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  immCommandAllocateInfo.pNext = nullptr;
  immCommandAllocateInfo.commandPool = immCommandPool;
  immCommandAllocateInfo.commandBufferCount = 1;
  immCommandAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  vkAllocateCommandBuffers(device, &immCommandAllocateInfo, &immCommandBuffer);

  deletionQueue.PushFunction(
      [&]() { vkDestroyCommandPool(device, immCommandPool, nullptr); });
}

void VulkanEngine::initSyncStructures() {
  VkFenceCreateInfo fenceCreateInfo =
      VulkanInit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphoreCreateInfo = VulkanInit::SemaphoreCreateInfo();

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence);

    vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                      &frames[i].swapchainSemaphore);
  }

  renderSemaphores.resize(swapchainImages.size());

  for (uint32_t i = 0; i < swapchainImages.size(); i++) {
    vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                      &renderSemaphores[i]);
  }

  vkCreateFence(device, &fenceCreateInfo, nullptr, &immFence);
  deletionQueue.PushFunction(
      [&]() { vkDestroyFence(device, immFence, nullptr); });
}

void VulkanEngine::initDescriptors() {
  // create a descriptor pool that will hold 10 sets with 1 image each
  std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

  globalDescriptorAllocator.InitPool(device, 10, sizes);

  // TODO: need to experiment on adding another binding for texture on a
  // different index later

  // make the descriptor set layout for our compute draw
  {
    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    drawImageDescriptorLayout =
        builder.Build(device, VK_SHADER_STAGE_COMPUTE_BIT);
  }

  // allocate a descriptor set for our draw image
  drawImageDescriptors =
      globalDescriptorAllocator.Allocate(device, drawImageDescriptorLayout);

  VkDescriptorImageInfo imgInfo{};
  imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  imgInfo.imageView = drawImage.imageView;

  VkWriteDescriptorSet drawImageWrite = {};
  drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  drawImageWrite.pNext = nullptr;

  drawImageWrite.dstBinding = 0;
  drawImageWrite.dstSet = drawImageDescriptors;
  drawImageWrite.descriptorCount = 1;
  drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  drawImageWrite.pImageInfo = &imgInfo;

  vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);

  // make sure both the descriptor allocator and the new layout get cleaned up
  // properly
  deletionQueue.PushFunction([&]() {
    globalDescriptorAllocator.DestroyPool(device);

    vkDestroyDescriptorSetLayout(device, drawImageDescriptorLayout, nullptr);
  });
}

void VulkanEngine::initPipelines() { initBackgroundPipelines(); }

void VulkanEngine::initBackgroundPipelines() {
  VkPipelineLayoutCreateInfo computeLayout{};
  computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  computeLayout.pNext = nullptr;
  computeLayout.pSetLayouts = &drawImageDescriptorLayout;
  computeLayout.setLayoutCount = 1;

  vkCreatePipelineLayout(device, &computeLayout, nullptr,
                         &gradientPipelineLayout);

  VkShaderModule computeDrawShader;
  if (!VulkanPipelines::LoadShaderModule("./shaders/gradient.comp.spv", device,
                                         &computeDrawShader)) {
    fmt::println("Error when building the compute shader \n");
  }

  VkPipelineShaderStageCreateInfo stageinfo{};
  stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stageinfo.pNext = nullptr;
  stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stageinfo.module = computeDrawShader;
  stageinfo.pName = "main";

  VkComputePipelineCreateInfo computePipelineCreateInfo{};
  computePipelineCreateInfo.sType =
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  computePipelineCreateInfo.pNext = nullptr;
  computePipelineCreateInfo.layout = gradientPipelineLayout;
  computePipelineCreateInfo.stage = stageinfo;

  vkCreateComputePipelines(device, VK_NULL_HANDLE, 1,
                           &computePipelineCreateInfo, nullptr,
                           &gradientPipeline);
  vkDestroyShaderModule(device, computeDrawShader, nullptr);

  deletionQueue.PushFunction([&]() {
    vkDestroyPipelineLayout(device, gradientPipelineLayout, nullptr);
    vkDestroyPipeline(device, gradientPipeline, nullptr);
  });
}

void VulkanEngine::immediateSubmit(
    std::function<void(VkCommandBuffer cmd)> &&function) {
  vkResetFences(device, 1, &immFence);
  vkResetCommandBuffer(immCommandBuffer, 0);

  VkCommandBuffer cmd = immCommandBuffer;

  VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::CommandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  function(cmd);

  vkEndCommandBuffer(cmd);

  VkCommandBufferSubmitInfo cmdinfo = VulkanInit::CommandBufferSubmitInfo(cmd);
  VkSubmitInfo2 submit = VulkanInit::SubmitInfo(&cmdinfo, nullptr, nullptr);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  vkQueueSubmit2(graphicsQueue, 1, &submit, immFence);

  vkWaitForFences(device, 1, &immFence, true, 9999999999);
}

void VulkanEngine::initImgui() {
  // 1: create descriptor pool for IMGUI
  //  the size of the pool is very oversize, but it's copied from imgui demo
  //  itself.
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  VkDescriptorPool imguiPool;
  vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool);

  // 2: initialize imgui library

  // this initializes the core structures of imgui
  ImGui::CreateContext();

  // this initializes imgui for SDL
  ImGui_ImplSDL2_InitForVulkan(window);

  // this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = instance;
  initInfo.PhysicalDevice = chosenGPU;
  initInfo.Device = device;
  initInfo.Queue = graphicsQueue;
  initInfo.DescriptorPool = imguiPool;
  initInfo.MinImageCount = 3;
  initInfo.ImageCount = 3;
  initInfo.UseDynamicRendering = true;

  // dynamic rendering parameters for imgui to use
  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount =
      1;
  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo
      .pColorAttachmentFormats = &swapchainImageFormat;
  initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_Init(&initInfo);

  // ImGui_ImplVulkan_CreateFontsTexture();

  // add the destroy the imgui created structures
  deletionQueue.PushFunction([&]() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
  });
}

void VulkanEngine::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
  VkRenderingAttachmentInfo colorAttachment = VulkanInit::AttachmentInfo(
      targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkRenderingInfo renderInfo =
      VulkanInit::RenderingInfo(swapchainExtent, &colorAttachment, nullptr);

  vkCmdBeginRendering(cmd, &renderInfo);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

  vkCmdEndRendering(cmd);
}
