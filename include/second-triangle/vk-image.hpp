#pragma once
#include "vulkan/vulkan_core.h"
class VulkanImage {
 public:
  static void TransitionImage(VkCommandBuffer cmd, VkImage image,
                              VkImageLayout currentLayout,
                              VkImageLayout newLayout);
};
