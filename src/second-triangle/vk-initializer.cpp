#include "second-triangle/vk-initializer.hpp"
#include "vulkan/vulkan_core.h"

VkFenceCreateInfo VulkanInit::FenceCreateInfo(VkFenceCreateFlags flags) {
  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = nullptr;

  fenceInfo.flags = flags;

  return fenceInfo;
}

VkSemaphoreCreateInfo VulkanInit::SemaphoreCreateInfo(
    VkSemaphoreCreateFlags flags) {
  VkSemaphoreCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = nullptr;

  info.flags = flags;

  return info;
}

VkCommandBufferBeginInfo VulkanInit::CommandBufferBeginInfo(
    VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;

  info.pInheritanceInfo = nullptr;
  info.flags = flags;

  return info;
}

VkImageSubresourceRange VulkanInit::ImageSubresourceRange(
    VkImageAspectFlags aspectMask) {
  VkImageSubresourceRange subImage{};
  subImage.aspectMask = aspectMask;
  subImage.baseMipLevel = 0;
  subImage.levelCount = VK_REMAINING_MIP_LEVELS;
  subImage.baseArrayLayer = 0;
  subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

  return subImage;
}
