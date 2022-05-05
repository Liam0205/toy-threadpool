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
  auto f = pool.async([]() -> int { return 0; });
  REQUIRE(f.get() == 0);
}

TEST_CASE("threadpool - run") {
  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.async([](int i) -> int { return i; }, i));
  }
  for (int i = 0; i != 30; ++i) {
    bool_futs.emplace_back(pool.async([](int i) -> bool { return i % 2 == 0; }, i));
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
    int_futs.emplace_back(pool.async([](int i) -> int { return i; }, i));
  }
  for (int i = 0; i != 30; ++i) {
    bool_futs.emplace_back(pool.async([i]() -> bool { return i % 2 == 0; }));
  }
  pool.terminate();
  REQUIRE(pool.inited());
  REQUIRE_FALSE(pool.is_running());
  for (int i = 0; i != 30; ++i) {
    REQUIRE(int_futs[i].get() == i);
    REQUIRE(bool_futs[i].get() == (i % 2 == 0));
  }
}

TEST_CASE("threadpool - terminate before async") {
  // Seed with a real random value, if available
  std::random_device r;

  // Choose a random mean between 1 and 6
  std::default_random_engine eng(r());
  std::uniform_int_distribution<int> uniform_dist(1, 100);

  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.async(
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
    REQUIRE_THROWS(bool_futs.emplace_back(pool.async([i]() -> bool { return i % 2 == 0; })));
  }
  for (int i = 0; i != 30; ++i) {
    REQUIRE(int_futs[i].get() == i);
  }
}

TEST_CASE("threadpool - cancel before get") {
  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.async([](int i) -> int { return i; }, i));
  }
  for (int i = 0; i != 30; ++i) {
    bool_futs.emplace_back(pool.async([i]() -> bool { return i % 2 == 0; }));
  }
  pool.cancel();
  REQUIRE(pool.inited());
  REQUIRE_FALSE(pool.is_running());
  int cancelled_int = 0;
  int cancelled_bool = 0;
  for (int i = 0; i != 30; ++i) {
    if (int_futs[i].valid() &&
        int_futs[i].wait_for(std::chrono::milliseconds(50)) == std::future_status::ready) {
      try {
        int ret = int_futs[i].get();
        REQUIRE(i == ret);
      } catch (const std::future_error& e) {
        // std::cerr << "Cancelled int_futs[" << i << "]: " << e.what() << std::endl;
        ++cancelled_int;
      }
    }
    if (bool_futs[i].valid() &&
        bool_futs[i].wait_for(std::chrono::milliseconds(50)) == std::future_status::ready) {
      try {
        bool ret = bool_futs[i].get();
        REQUIRE((i % 2 == 0) == ret);
      } catch (const std::future_error& e) {
        // std::cerr << "Cancelled bool_futs[" << i << "]: " << e.what() << std::endl;
        ++cancelled_bool;
      }
    }
  }
  std::cerr << "Cancelled int: " << cancelled_int << "; cancelled bool: " << cancelled_bool
            << std::endl;
}

TEST_CASE("threadpool - cancel before async") {
  // Seed with a real random value, if available
  std::random_device r;

  // Choose a random mean between 1 and 6
  std::default_random_engine eng(r());
  std::uniform_int_distribution<int> uniform_dist(1, 100);

  yuuki::threadpool pool;
  pool.init(3);
  std::vector<std::future<int>> int_futs;
  std::vector<std::future<bool>> bool_futs;
  for (int i = 0; i != 30; ++i) {
    int_futs.emplace_back(pool.async(
        [&eng, &uniform_dist](int i) -> int {
          std::this_thread::sleep_for(std::chrono::milliseconds(uniform_dist(eng)));
          return i;
        },
        i));
  }
  pool.cancel();
  REQUIRE(pool.inited());
  REQUIRE_FALSE(pool.is_running());
  for (int i = 0; i != 30; ++i) {
    REQUIRE_THROWS(bool_futs.emplace_back(pool.async([i]() -> bool { return i % 2 == 0; })));
  }
  int cancelled_int = 0;
  for (int i = 0; i != 30; ++i) {
    if (int_futs[i].valid() &&
        int_futs[i].wait_for(std::chrono::milliseconds(50)) == std::future_status::ready) {
      try {
        int ret = int_futs[i].get();
        REQUIRE(i == ret);
      } catch (const std::future_error& e) {
        // std::cerr << "Cancelled [" << i << "]: " << e.what() << std::endl;
        ++cancelled_int;
      }
    }
  }
  std::cerr << "Cancelled int: " << cancelled_int << std::endl;
}
