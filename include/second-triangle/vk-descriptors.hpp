#pragma once
#include <span>
#include <vector>
#include "vulkan/vulkan_core.h"
class DescriptorLayoutBuilder {
 private:
  std::vector<VkDescriptorSetLayoutBinding> bindings;

 public:
  void AddBinding(uint32_t binding, VkDescriptorType type);
  void Clear();
  VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages,
                              void* pNext = nullptr,
                              VkDescriptorSetLayoutCreateFlags flags = 0);
};

class DescriptorAllocator {
 private:
  VkDescriptorPool pool;

 public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    float ratio;
  };
  void InitPool(VkDevice device, uint32_t maxSets,
                std::span<PoolSizeRatio> poolRatios);
  void ClearDescriptors(VkDevice device);
  void DestroyPool(VkDevice device);

  VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);
};
