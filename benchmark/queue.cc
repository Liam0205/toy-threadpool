// =========================================================================
// Copyright 2021 -- present Liam Huang (Yuuki) [liamhuang0205@gmail.com].
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

#include <catch.h>
#include <yuuki/blocking_queue.h>
#include <yuuki/threadpool.h>
#include <yuuki/threadsafe_queue.h>

#include <cstdint>
#include <future>
#include <thread>

const int NUM = 2048;

TEST_CASE("queue - multi_push") {
  yuuki::threadpool<yuuki::threadsafe_queue<std::function<void()>>> pool;
  pool.init(std::thread::hardware_concurrency());

  yuuki::blocking_queue<int> b_queue;
  BENCHMARK("b_queue") {
    std::vector<std::future<void>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(
          pool.async([&b_queue, i]() -> void { return b_queue.push(i); }));
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };

  yuuki::threadsafe_queue<int> ts_queue;
  BENCHMARK("ts_queue") {
    std::vector<std::future<void>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(
          pool.async([&ts_queue, i]() -> void { return ts_queue.push(i); }));
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };
}

TEST_CASE("queue - multi_push_pop") {
  yuuki::threadpool<yuuki::threadsafe_queue<std::function<void()>>> pool;
  pool.init(std::thread::hardware_concurrency());

  yuuki::blocking_queue<int> b_queue;
  BENCHMARK("b_queue") {
    std::vector<std::future<void>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(
          pool.async([&b_queue, i]() -> void { return b_queue.push(i); }));
      if (i % 2 != 0) {
        futs.emplace_back(pool.async([&b_queue, i]() -> void {
          int holder;
          b_queue.pop(holder);
          return;
        }));
      }
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };

  yuuki::threadsafe_queue<int> ts_queue;
  BENCHMARK("ts_queue") {
    std::vector<std::future<void>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(
          pool.async([&ts_queue, i]() -> void { return ts_queue.push(i); }));
      if (i % 2 != 0) {
        futs.emplace_back(pool.async([&ts_queue, i]() -> void {
          int holder;
          ts_queue.pop(holder);
          return;
        }));
      }
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };
}
