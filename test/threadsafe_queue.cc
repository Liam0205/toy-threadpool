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
#include <yuuki/threadsafe_queue.h>

#include <cstdint>
#include <future>
#include <vector>

TEST_CASE("single-thread usage") {
  yuuki::threadsafe_queue<uint64_t> q;
  REQUIRE(q.empty());
  q.emplace(42UL);
  REQUIRE_FALSE(q.empty());
  q.clear();
  REQUIRE(q.empty());
  for (uint64_t i = 0; i != 42UL; ++i) {
    q.emplace(i);
  }
  REQUIRE_FALSE(q.empty());
  REQUIRE(q.size() == 42);
  for (uint64_t i = 0; i != 42UL; ++i) {
    uint64_t holder;
    REQUIRE(q.pop(holder));
    REQUIRE(holder == i);
  }
}

TEST_CASE("multiple-thread push") {
  yuuki::threadsafe_queue<uint64_t> q;
  REQUIRE(q.empty());
  std::vector<std::future<void>> futures;
  futures.reserve(42);
  for (uint64_t i = 0; i != 42UL; ++i) {
    futures.emplace_back(std::async(
        std::launch::async, &yuuki::threadsafe_queue<uint64_t>::push, &q, i));
  }
  for (auto& fut : futures) {
    fut.get();
  }
  REQUIRE(q.size() == 42UL);
  for (uint64_t i = 0; i != 42UL; ++i) {
    uint64_t holder;
    REQUIRE(q.pop(holder));
    REQUIRE(holder >= 0);
    REQUIRE(holder < 42);
  }
}

TEST_CASE("multiple-thread push-pop") {
  yuuki::threadsafe_queue<uint64_t> q;
  REQUIRE(q.empty());
  std::vector<std::future<void>> push_futures;
  push_futures.reserve(42);
  for (uint64_t i = 0; i != 42UL; ++i) {
    push_futures.emplace_back(std::async(
        std::launch::async, &yuuki::threadsafe_queue<uint64_t>::push, &q, i));
  }
  for (auto& fut : push_futures) {
    fut.get();
  }
  REQUIRE(q.size() == 42UL);
  std::vector<std::future<bool>> pop_futures;
  for (uint64_t i = 0; i != 42UL; ++i) {
    uint64_t holder;
    pop_futures.emplace_back(std::async(std::launch::async,
                                        &yuuki::threadsafe_queue<uint64_t>::pop,
                                        &q, std::ref(holder)));
  }
  for (auto& fut : pop_futures) {
    REQUIRE(fut.get());
  }
}
