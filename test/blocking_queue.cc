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

#include <cstdint>
#include <future>
#include <vector>

TEST_CASE("single-thread usage") {
  yuuki::blocking_queue<uint64_t> q;
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
}

TEST_CASE("multiple-thread usage") {
  yuuki::blocking_queue<uint64_t> q;
  REQUIRE(q.empty());
  std::vector<std::future<void>> futures;
  futures.reserve(42);
  for (uint64_t i = 0; i != 42UL; ++i) {
    futures.emplace_back(std::async(
        std::launch::async, &yuuki::blocking_queue<uint64_t>::push, &q, i));
  }
  for (auto& fut : futures) {
    fut.get();
  }
  REQUIRE(q.size() == 42UL);
  while (!q.empty()) {
    uint64_t holder;
    REQUIRE(q.pop(holder));
    REQUIRE(0 <= holder);
    REQUIRE(holder < 42UL);
  }
}
