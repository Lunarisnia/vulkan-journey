#include "second-triangle/second-triangle.hpp"
#include <cstdlib>
#include <exception>
#include "fmt/base.h"
#include "second-triangle/vk-engine.hpp"

void SecondTriangle::Run() {
  try {
    vkEngine.Init();
    /*vkEngine.Run();*/
  } catch (std::exception& error) {
    fmt::println("Error: {}", error.what());
    throw error;
  }
}

SecondTriangle::~SecondTriangle() {
  fmt::println("Cleaning up!");
  vkEngine.Cleanup();
}
