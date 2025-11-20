#include "second-triangle/deletion-queue.hpp"
#include <functional>

void DeletionQueue::PushFunction(std::function<void()>&& callback) {
  deletors.emplace_back(callback);
}

void DeletionQueue::Flush() {
  for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
    (*it)();  // call functors
  }

  deletors.clear();
}
