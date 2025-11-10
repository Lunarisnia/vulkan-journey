#include "first-triangle/app.hpp"
#include <exception>
#include <iostream>

int main() {
  try {
    FirstTriangle::App firstTriangleApp;
    firstTriangleApp.Run();
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
}
