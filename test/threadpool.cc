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
#include <yuuki/threadpool.h>

#include <chrono>
#include <cstdint>
#include <future>
#include <random>

std::uint64_t Fibonacci(std::uint64_t number) {
  return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}

TEST_CASE("threadpool - dryrun") {
  yuuki::threadpool pool;
  REQUIRE_FALSE(pool.inited());
  REQUIRE_FALSE(pool.is_running());
  pool.init(3);
  REQUIRE(pool.inited());
  REQUIRE(pool.is_running());
  REQUIRE(pool.size() == 3);
}

TEST_CASE("threadpool - testrun") {
  yuuki::threadpool pool;
  pool.init(3);
  auto f = pool.delegate([]() -> int { return 0; });
  REQUIRE(f.get() == 0);
}

TEST_CASE("threadpool - run") {
  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.delegate([](int i) -> int { return i; }, i));
  }
  for (int i = 0; i != 30; ++i) {
    bool_futs.emplace_back(pool.delegate([](int i) -> bool { return i % 2 == 0; }, i));
  }
  for (int i = 0; i != 30; ++i) {
    REQUIRE(int_futs[i].get() == i);
    REQUIRE(bool_futs[i].get() == (i % 2 == 0));
  }
}

TEST_CASE("threadpool - terminate before get") {
  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.delegate([](int i) -> int { return i; }, i));
  }
  for (int i = 0; i != 30; ++i) {
    bool_futs.emplace_back(pool.delegate([i]() -> bool { return i % 2 == 0; }));
  }
  pool.terminate();
  REQUIRE(pool.inited());
  REQUIRE_FALSE(pool.is_running());
  for (int i = 0; i != 30; ++i) {
    REQUIRE(int_futs[i].get() == i);
    REQUIRE(bool_futs[i].get() == (i % 2 == 0));
  }
}

TEST_CASE("threadpool - terminate before delegate") {
  // Seed with a real random value, if available
  std::random_device r;

  // Choose a random mean between 1 and 6
  std::default_random_engine eng(r());
  std::uniform_int_distribution<int> uniform_dist(1, 1000);

  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.delegate(
        [&eng, &uniform_dist](int i) -> int {
          std::this_thread::sleep_for(std::chrono::milliseconds(uniform_dist(eng)));
          return i;
        },
        i));
  }
  pool.terminate();
  REQUIRE(pool.inited());
  REQUIRE_FALSE(pool.is_running());
  for (int i = 0; i != 30; ++i) {
    REQUIRE_THROWS(bool_futs.emplace_back(pool.delegate([i]() -> bool { return i % 2 == 0; })));
  }
  for (int i = 0; i != 30; ++i) {
    REQUIRE(int_futs[i].get() == i);
  }
}

TEST_CASE("threadpool - benchmark") {
  yuuki::threadpool pool;
  pool.init(30);

  const int NUM = 10240;

  BENCHMARK("pool.async") {
    std::vector<std::future<uint64_t>> futs;
    futs.reserve(NUM);
    for (int i = 0; i != NUM; ++i) {
      futs.emplace_back(pool.async(Fibonacci, i % 10));
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
