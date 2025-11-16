#pragma once
#include "vulkan/vulkan_core.h"
class VulkanInit {
 public:
  static VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
  static VkSemaphoreCreateInfo SemaphoreCreateInfo(
      VkSemaphoreCreateFlags flags = 0);
  static VkCommandBufferBeginInfo CommandBufferBeginInfo(
      VkCommandBufferUsageFlags flags = 0);
  static VkImageSubresourceRange ImageSubresourceRange(
      VkImageAspectFlags aspectMask);
};
