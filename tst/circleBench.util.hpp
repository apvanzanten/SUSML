// This file is part of Still Untitled State Machine Library (SUSML).
//    Copyright (C) 2021 A.P. van Zanten
// SUSML is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// SUSML is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public License
// along with SUSML. If not, see <https://www.gnu.org/licenses/>.

#ifndef CIRCLEBENCH_UTIL_HPP
#define CIRCLEBENCH_UTIL_HPP

#include <benchmark/benchmark.h>

#include "common.hpp"
#include "factory.hpp"
#include "tuplebased.hpp"
#include "vectorbased.hpp"

namespace util {

enum class HasGuards { yes, no };

namespace tuplebased {
using susml::Transition;

template <std::size_t Index, std::size_t TotalTransitions, bool WithGuard = false>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr std::size_t source = Index;
  constexpr std::size_t target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  if constexpr (!WithGuard) {
    return Transition(source, target, true, susml::NoneType{}, [&] { counter += Index; });
  } else if constexpr (WithGuard) {
    return Transition(
        source, target, true, [&] { return ((counter++ & 1) == 0); }, [&] { counter += Index; });
  }
}

template <bool WithGuards, std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &, std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::make_tuple(makeTransition<Indices, totalTransitions, WithGuards>(counter)...);
}

template <std::size_t NumTransitions, bool WithGuards = false>
constexpr auto makeStateMachine(std::size_t &counter) {
  auto tuple = util::tuplebased::makeTransitions<WithGuards>(
      std::make_index_sequence<NumTransitions>(), counter);

  return susml::tuplebased::StateMachine<std::size_t, bool, decltype(tuple)>(0, tuple);
}

} // namespace tuplebased

namespace vectorbased {
using namespace susml::factory;

template <std::size_t Index, std::size_t TotalTransitions, bool WithGuard = false>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  const auto partial =
      From(source).To(target).On(true).Do(std::function([&] { counter += Index; }));

  if constexpr (WithGuard) {
    return partial.If(std::function([&] { return ((counter++ & 1) == 0); })).make();
  } else if constexpr (!WithGuard) {
    return partial.make();
  }
}

template <bool WithGuards, std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &, std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::vector{makeTransition<Indices, totalTransitions, WithGuards>(counter)...};
}

template <std::size_t NumTransitions, bool WithGuards = false>
constexpr auto makeStateMachine(std::size_t &counter) {
  std::vector transitions = util::vectorbased::makeTransitions<WithGuards>(
      std::make_index_sequence<NumTransitions>(), counter);

  using TransitionContainer = decltype(transitions);
  using Transition          = typename TransitionContainer::value_type;

  return susml::vectorbased::StateMachine<Transition>{0, transitions};
}
} // namespace vectorbased

template <typename StateMachine>
static void runTest(benchmark::State &s, StateMachine &machine, size_t &counter) {
  for (auto _ : s) {
    for (int i = 0; i < s.range(0); i++) {
      machine.trigger(true);
    }
  }
  s.counters["c"] = counter;
}

template <std::size_t NumTransitions, util::HasGuards hasGuards>
static void circleTupleBased(benchmark::State &s) {
  std::size_t counter = 0;
  auto        m =
      tuplebased::makeStateMachine<NumTransitions, (hasGuards == util::HasGuards::yes)>(counter);
  runTest(s, m, counter);
}

template <std::size_t NumTransitions, util::HasGuards hasGuards>
static void circleVectorBased(benchmark::State &s) {
  std::size_t counter = 0;
  auto        m =
      vectorbased::makeStateMachine<NumTransitions, (hasGuards == util::HasGuards::yes)>(counter);
  runTest(s, m, counter);
}

} // namespace util

#define BENCH_CIRCLE(NumTransitions, HasGuards)                                                    \
  namespace {                                                                                      \
  using util::circleTupleBased;                                                                    \
  using util::circleVectorBased;                                                                   \
  BENCHMARK_TEMPLATE(circleTupleBased, NumTransitions, HasGuards)                                  \
      ->Arg(100000)                                                                                \
      ->Unit(benchmark::kMicrosecond);                                                             \
  BENCHMARK_TEMPLATE(circleVectorBased, NumTransitions, HasGuards)                                 \
      ->Arg(100000)                                                                                \
      ->Unit(benchmark::kMicrosecond);                                                             \
  }

#endif