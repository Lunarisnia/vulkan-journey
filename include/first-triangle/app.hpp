#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FirstTriangle {

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

class App {
 private:
  GLFWwindow* window;
  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;

 public:
  void Run();

 private:
  void initWindow();
  void initVulkan();
  void createInstance();
  void mainLoop();
  void cleanup();

 public:
  ~App();
};
};  // namespace FirstTriangle
