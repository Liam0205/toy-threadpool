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

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace yuuki {
class threadpool {
 public:
  threadpool() = default;
  ~threadpool();
  threadpool(const threadpool&) = delete;
  threadpool(threadpool&&) = delete;
  threadpool& operator=(const threadpool&) = delete;
  threadpool& operator=(threadpool&&) = delete;

 public:
  void init(int num);
  void terminate();

 public:
  bool inited() const;
  bool is_running() const;
  int size() const;

 private:
  void spawn();

 public:
  template <class F, class... Args>
  auto delegate(F&& f, Args&&... args) const -> std::future<decltype(f(args...))>;
  template <class F, class... Args>
  inline auto async(F&& f, Args&&... args) const -> std::future<decltype(f(args...))> {
    return delegate(std::forward<F>(f), std::forward<Args>(args)...);
  }

 private:
  bool inited_{false};
  bool stop_{false};
  std::vector<std::thread> workers_;
  mutable std::queue<std::function<void()>> tasks_;
  mutable std::mutex mtx_;
  mutable std::condition_variable cond_;
  mutable std::once_flag once_;
};

inline threadpool::~threadpool() {
  terminate();
}

inline void threadpool::init(int num) {
  std::call_once(once_, [this, num]() {
    std::unique_lock<std::mutex> lock(mtx_);
    stop_ = false;
    workers_.reserve(num);
    for (int i = 0; i < num; ++i) {
      workers_.emplace_back(std::bind(&threadpool::spawn, this));
    }
    inited_ = true;
  });
}

inline void threadpool::terminate() {
  {
    std::unique_lock<std::mutex> lock(mtx_);
    if (inited_ && !stop_) {
      stop_ = true;
    } else {
      return;
    }
  }
  cond_.notify_all();
  for (auto& worker : workers_) {
    worker.join();
  }
}

inline bool threadpool::inited() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return inited_;
}

inline bool threadpool::is_running() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return inited_ && !stop_;
}

inline int threadpool::size() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return workers_.size();
}

inline void threadpool::spawn() {
  for (;;) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(this->mtx_);
      this->cond_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });

      if (stop_ && tasks_.empty()) return;
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}

template <class F, class... Args>
auto threadpool::delegate(F&& f, Args&&... args) const -> std::future<decltype(f(args...))> {
  using return_t = decltype(f(args...));
  using future_t = std::future<return_t>;
  using task_t = std::packaged_task<return_t()>;

  auto bind_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  std::shared_ptr<task_t> task = std::make_shared<task_t>(std::move(bind_func));
  future_t fut = task->get_future();
  {
    std::unique_lock<std::mutex> lock(mtx_);
    if (stop_) throw std::runtime_error("delegating task to a threadpool that has stopped.");
    tasks_.emplace([task]() -> void { (*task)(); });
  }
  cond_.notify_one();
  return fut;
}
}  // namespace yuuki
