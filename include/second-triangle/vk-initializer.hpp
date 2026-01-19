#pragma once
#include "vulkan/vulkan_core.h"
class VulkanInit {
public:
  static VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
  static VkSemaphoreCreateInfo
  SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
  static VkCommandBufferBeginInfo
  CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
  static VkImageSubresourceRange
  ImageSubresourceRange(VkImageAspectFlags aspectMask);
  static VkSemaphoreSubmitInfo
  SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
  static VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);
  static VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo *cmd,
                                  VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                                  VkSemaphoreSubmitInfo *waitSemaphoreInfo);
  static VkImageCreateInfo ImageCreateInfo(VkFormat format,
                                           VkImageUsageFlags usageFlags,
                                           VkExtent3D extent);
  static VkImageViewCreateInfo
  ImageViewCreateInfo(VkFormat format, VkImage image,
                      VkImageAspectFlags aspectFlags);
  static VkRenderingAttachmentInfo AttachmentInfo(
      VkImageView view, VkClearValue *clear,
      VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/);
  static VkRenderingInfo
  RenderingInfo(VkExtent2D renderExtent,
                VkRenderingAttachmentInfo *colorAttachment,
                VkRenderingAttachmentInfo *depthAttachment);
};
