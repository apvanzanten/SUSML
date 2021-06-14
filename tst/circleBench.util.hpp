#ifndef CIRCLEBENCH_UTIL_HPP
#define CIRCLEBENCH_UTIL_HPP

#include <benchmark/benchmark.h>

#include "minimal/factory.hpp"
#include "tuplebased/StateMachine.hpp"

namespace util {

namespace tuplebased {
using susml::tuplebased::Transition;

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr std::size_t source = Index;
  constexpr std::size_t target =
      ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  return Transition(source, target, true, std::make_tuple(),
                    std::make_tuple([&] { counter += Index; }));
}

template <std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &,
                               std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::make_tuple(makeTransition<Indices, totalTransitions>(counter)...);
}

template <std::size_t NumTransitions>
constexpr auto makeStateMachine(std::size_t &counter) {
  auto tuple = util::tuplebased::makeTransitions(
      std::make_index_sequence<NumTransitions>(), counter);

  return susml::tuplebased::StateMachine<std::size_t, bool, decltype(tuple)>(
      tuple, 0);
}

} // namespace tuplebased

namespace minimal {
using namespace susml::minimal::factory;

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  return From(source)
      .To(target)
      .On(true)
      .Do({std::function([&] { counter += Index; })})
      .make();
}

template <std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &,
                               std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::vector{makeTransition<Indices, totalTransitions>(counter)...};
}

template <std::size_t NumTransitions>
constexpr auto makeStateMachine(std::size_t &counter) {
  std::vector transitions = util::minimal::makeTransitions(
      std::make_index_sequence<NumTransitions>(), counter);

  using TransitionContainer = decltype(transitions);
  using Transition = typename TransitionContainer::value_type;

  return susml::minimal::StateMachine<Transition, TransitionContainer>{
      transitions, 0};
}
} // namespace minimal

template <typename StateMachine>
static void runTest(benchmark::State &s, StateMachine &machine,
                    size_t &counter) {
  for (auto _ : s) {
    for (int i = 0; i < s.range(0); i++) {
      machine.trigger(true);
    }
  }
  s.counters["c"] = counter;
}

template <std::size_t NumTransitions>
static void benchTupleBased(benchmark::State &s) {
  std::size_t counter = 0;
  auto m = tuplebased::makeStateMachine<NumTransitions>(counter);
  runTest(s, m, counter);
}

template <std::size_t NumTransitions>
static void benchMinimal(benchmark::State &s) {
  std::size_t counter = 0;
  auto m = minimal::makeStateMachine<NumTransitions>(counter);
  runTest(s, m, counter);
}
} // namespace util

#define BENCH_CIRCLE(NumTransitions)                                           \
  namespace {                                                                  \
  using util::benchTupleBased;                                                 \
  using util::benchMinimal;                                                    \
  BENCHMARK_TEMPLATE(benchTupleBased, NumTransitions)                          \
      ->Arg(100000)                                                            \
      ->Unit(benchmark::kMicrosecond);                                         \
  BENCHMARK_TEMPLATE(benchMinimal, NumTransitions)                             \
      ->Arg(100000)                                                            \
      ->Unit(benchmark::kMicrosecond);                                         \
  }

#endif