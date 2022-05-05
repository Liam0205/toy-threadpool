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

#include <cstdint>

std::uint64_t Fibonacci(std::uint64_t number) {
  return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}

TEST_CASE("Fibonacci") {
  REQUIRE(Fibonacci(0) == 1);
  REQUIRE_FALSE(Fibonacci(5) == 7);  // actural should be `8`.

  // now let's benchmark:
  BENCHMARK("Fibonacci 10") {
    return Fibonacci(10);
  };

  BENCHMARK("Fibonacci 25") {
    return Fibonacci(25);
  };

  BENCHMARK("Fibonacci 30") {
    return Fibonacci(30);
  };

  BENCHMARK("Fibonacci 35") {
    return Fibonacci(35);
  };
}
