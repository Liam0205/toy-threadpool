// =========================================================================
// Copyright 2022 -- present Liam Huang (Yuuki) [liamhuang0205@gmail.com].
// Author: Liam Huang
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =========================================================================

#pragma once

#include <queue>
#include <shared_mutex>

namespace yuuki {
template <typename T>
class blocking_queue : protected std::queue<T> {
 public:
  using wlock = std::unique_lock<std::shared_mutex>;
  using rlock = std::shared_lock<std::shared_mutex>;

 public:
  blocking_queue() = default;
  ~blocking_queue() {
    clear();
  }
  blocking_queue(const blocking_queue&) = delete;
  blocking_queue(blocking_queue&&) = delete;
  blocking_queue& operator=(const blocking_queue&) = delete;
  blocking_queue& operator=(blocking_queue&&) = delete;

 public:
  bool empty() const {
    rlock lock(mtx_);
    return std::queue<T>::empty();
  }

  size_t size() const {
    rlock lock(mtx_);
    return std::queue<T>::size();
  }

 public:
  void clear() {
    wlock lock(mtx_);
    while (!std::queue<T>::empty()) {
      std::queue<T>::pop();
    }
  }

  void push(const T& obj) {
    wlock lock(mtx_);
    std::queue<T>::push(obj);
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    wlock lock(mtx_);
    std::queue<T>::emplace(std::forward<Args>(args)...);
  }

  bool pop(T& holder) {
    wlock lock(mtx_);
    if (std::queue<T>::empty()) {
      return false;
    } else {
      holder = std::move(std::queue<T>::front());
      std::queue<T>::pop();
      return true;
    }
  }

 private:
  mutable std::shared_mutex mtx_;
};
}  // namespace yuuki
