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

std::uint64_t Fibonacci(std::uint64_t number) {
  return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}

TEST_CASE("threadpool - benchmark") {
  const int NUM = 10240;

  yuuki::threadpool<yuuki::blocking_queue<std::function<void()>>> b_queue;
  b_queue.init(30);
  BENCHMARK("b_queue.async") {
    std::vector<std::future<uint64_t>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(b_queue.async(Fibonacci, i % 10));
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };

  yuuki::threadpool<yuuki::threadsafe_queue<std::function<void()>>> ts_queue;
  ts_queue.init(30);
  BENCHMARK("ts_queue.async") {
    std::vector<std::future<uint64_t>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(ts_queue.async(Fibonacci, i % 10));
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };

  BENCHMARK("std::async") {
    std::vector<std::future<uint64_t>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(std::async(std::launch::async, Fibonacci, i % 10));
    }
    for (auto& fut : futs) {
      fut.get();
    }
    return;
  };
}
