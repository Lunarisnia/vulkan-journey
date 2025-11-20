#pragma once
#include <deque>
#include <functional>
class DeletionQueue {
 private:
  std::deque<std::function<void()>> deletors;

 public:
  void PushFunction(std::function<void()>&& callback);
  void Flush();
};
