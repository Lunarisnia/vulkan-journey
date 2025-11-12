#include "second-triangle/second-triangle.hpp"
#include <exception>
#include "fmt/base.h"
#include "second-triangle/vk-engine.hpp"

void SecondTriangle::Run() {
  try {
    VulkanEngine vkEngine;
    vkEngine.Init();
  } catch (std::exception& error) {
    fmt::println("Error: {}", error.what());
    throw error;
  }
}
