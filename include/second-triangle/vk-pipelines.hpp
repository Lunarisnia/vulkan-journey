#pragma once
#include "vulkan/vulkan_core.h"
class VulkanPipelines {
 public:
  static bool LoadShaderModule(const char* filePath, VkDevice device,
                               VkShaderModule* outShaderModule);
};
