#include "first-triangle/app.hpp"
#include <cstdint>
#include <cstring>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

void FirstTriangle::App::Run() {
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

void FirstTriangle::App::initWindow() {
  glfwInit();

  // Tell glfw to not create an OpenGL Context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // No resize for now
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Hello, Triangle", nullptr, nullptr);
}

void FirstTriangle::App::initVulkan() { createInstance(); }

void FirstTriangle::App::createInstance() {
  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "Hello, Triangle",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14,
  };

  // Get the required instance extensions from GLFW.
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<vk::ExtensionProperties> supportedExtensions =
      context.enumerateInstanceExtensionProperties();

  // Check if all the required extension is supported
  std::vector<const char*> requiredExtensions;
  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    bool found = false;
    for (const vk::ExtensionProperties& ext : supportedExtensions) {
      if (strcmp(ext.extensionName, glfwExtensions[i]) == 0) {
        found = true;
        requiredExtensions.emplace_back(ext.extensionName);
        break;
      }
    }

    if (!found) {
      throw std::runtime_error("Required GLFW extension not supported: " +
                               std::string(glfwExtensions[i]));
    }
  }
  requiredExtensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);

  std::vector<const char*> requiredLayers;
  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  std::vector<vk::LayerProperties> supportedLayers =
      context.enumerateInstanceLayerProperties();
  for (const char*& requiredLayer : requiredLayers) {
    bool found = false;
    for (const vk::LayerProperties& layer : supportedLayers) {
      if (strcmp(requiredLayer, layer.layerName) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      throw std::runtime_error("Required validation layers not supported: " +
                               std::string(requiredLayer));
    }
  }

  vk::InstanceCreateInfo createInfo{
      .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = (uint32_t)requiredExtensions.size(),
      .ppEnabledExtensionNames = requiredExtensions.data(),
  };

  try {
    instance = vk::raii::Instance(context, createInfo);
  } catch (const vk::SystemError& err) {
    throw std::runtime_error("vulkan error: " + std::string(err.what()));
  }
}

void FirstTriangle::App::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void FirstTriangle::App::cleanup() {
  glfwDestroyWindow(window);

  glfwTerminate();
}

FirstTriangle::App::~App() { std::println("Cleaning up"); }
