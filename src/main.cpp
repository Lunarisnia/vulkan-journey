#include "second-triangle/second-triangle.hpp"
#include <exception>
#include <iostream>

int main() {
  try {
    SecondTriangle secondTriangleApp;
    secondTriangleApp.Run();
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
}
