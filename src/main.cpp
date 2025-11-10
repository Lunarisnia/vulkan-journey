#include "first-triangle/app.hpp"
#include "print"
#include <exception>
#include <print>

int main() {
  try {
    FirstTriangle::App firstTriangleApp;
    firstTriangleApp.Init();
  } catch (const std::exception &error) {
    std::println("%s", error.what());
  }
}
