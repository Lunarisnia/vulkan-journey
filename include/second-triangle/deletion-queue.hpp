#pragma once
#include <deque>
#include <functional>
#include <vector>
class DeletionQueue {
 private:
  /*std::deque<std::function<void()>> deletors;*/
  std::vector<std::function<void()>> deletors;

 public:
  void PushFunction(std::function<void()>&& callback);
  void Flush();
};
