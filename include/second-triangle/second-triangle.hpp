#pragma once
#include "second-triangle/vk-engine.hpp"
class SecondTriangle {
 private:
  VulkanEngine vkEngine;

 public:
  void Run();

 public:
  ~SecondTriangle();
};
