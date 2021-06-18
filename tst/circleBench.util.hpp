#ifndef CIRCLEBENCH_UTIL_HPP
#define CIRCLEBENCH_UTIL_HPP

#include <benchmark/benchmark.h>

#include "Transition.hpp"
#include "dataoriented/StateMachine.hpp"
#include "factory.hpp"
#include "minimal/StateMachine.hpp"
#include "tuplebased/StateMachine.hpp"

namespace util {

enum class HasGuards { yes, no };

namespace tuplebased {
using susml::Transition;

template <std::size_t Index, std::size_t TotalTransitions,
          bool WithGuard = false>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr std::size_t source = Index;
  constexpr std::size_t target =
      ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  if constexpr (!WithGuard) {
    return Transition(
        source, target, true, [] { return true; }, [&] { counter += Index; });
  } else if constexpr (WithGuard) {
    return Transition(
        source, target, true, [&] { return ((counter++ & 1) == 0); },
        [&] { counter += Index; });
  }
}

template <bool WithGuards, std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &,
                               std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::make_tuple(
      makeTransition<Indices, totalTransitions, WithGuards>(counter)...);
}

template <std::size_t NumTransitions, bool WithGuards = false>
constexpr auto makeStateMachine(std::size_t &counter) {
  auto tuple = util::tuplebased::makeTransitions<WithGuards>(
      std::make_index_sequence<NumTransitions>(), counter);

  return susml::tuplebased::StateMachine<std::size_t, bool, decltype(tuple)>(
      tuple, 0);
}

} // namespace tuplebased

namespace minimal {
using namespace susml::factory;

template <std::size_t Index, std::size_t TotalTransitions,
          bool WithGuard = false>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  const auto partial = From(source).To(target).On(true).Do(
      std::function([&] { counter += Index; }));

  if constexpr (WithGuard) {
    return partial.If(std::function([&] { return ((counter++ & 1) == 0); }))
        .make();
  } else if constexpr (!WithGuard) {
    return partial.make();
  }
}

template <bool WithGuards, std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &,
                               std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::vector{
      makeTransition<Indices, totalTransitions, WithGuards>(counter)...};
}

template <std::size_t NumTransitions, bool WithGuards = false>
constexpr auto makeStateMachine(std::size_t &counter) {
  std::vector transitions = util::minimal::makeTransitions<WithGuards>(
      std::make_index_sequence<NumTransitions>(), counter);

  using TransitionContainer = decltype(transitions);
  using Transition = typename TransitionContainer::value_type;

  return susml::minimal::StateMachine<Transition, TransitionContainer>{
      transitions, 0};
}
} // namespace minimal

namespace dataoriented {

template <bool WithGuards, std::size_t... Indices>
constexpr auto makeStateMachineImpl(const std::index_sequence<Indices...> &,
                                    std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return susml::dataoriented::fromTransitions(
      std::size_t(0), std::vector{minimal::makeTransition<Indices, totalTransitions, WithGuards>(counter)...});
}

template <std::size_t NumTransitions, bool WithGuards = false>
constexpr auto makeStateMachine(std::size_t &counter) {
  return makeStateMachineImpl<WithGuards>(
      std::make_index_sequence<NumTransitions>(), counter);
}
} // namespace dataoriented

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

template <std::size_t NumTransitions, util::HasGuards hasGuards>
static void circleTupleBased(benchmark::State &s) {
  std::size_t counter = 0;
  auto m = tuplebased::makeStateMachine<NumTransitions,
                                        (hasGuards == util::HasGuards::yes)>(
      counter);
  runTest(s, m, counter);
}

template <std::size_t NumTransitions, util::HasGuards hasGuards>
static void circleMinimal(benchmark::State &s) {
  std::size_t counter = 0;
  auto m =
      minimal::makeStateMachine<NumTransitions,
                                (hasGuards == util::HasGuards::yes)>(counter);
  runTest(s, m, counter);
}

template <std::size_t NumTransitions, util::HasGuards hasGuards>
static void circleDataOriented(benchmark::State &s) {
  std::size_t counter = 0;
  auto m = dataoriented::makeStateMachine<NumTransitions,
                                          (hasGuards == util::HasGuards::yes)>(
      counter);
  runTest(s, m, counter);
}

} // namespace util

#define BENCH_CIRCLE(NumTransitions, HasGuards)                                \
  namespace {                                                                  \
  using util::circleTupleBased;                                                \
  using util::circleMinimal;                                                   \
  using util::circleDataOriented;                                              \
  BENCHMARK_TEMPLATE(circleTupleBased, NumTransitions, HasGuards)              \
      ->Arg(100000)                                                            \
      ->Unit(benchmark::kMicrosecond);                                         \
  BENCHMARK_TEMPLATE(circleMinimal, NumTransitions, HasGuards)                 \
      ->Arg(100000)                                                            \
      ->Unit(benchmark::kMicrosecond);                                         \
  BENCHMARK_TEMPLATE(circleDataOriented, NumTransitions, HasGuards)            \
      ->Arg(100000)                                                            \
      ->Unit(benchmark::kMicrosecond);                                         \
  }

#endif