#include <benchmark/benchmark.h>
#include <tuple>
#include <type_traits>
#include <utility>

#include "factory.hpp"
#include "optimized.hpp"
#include "susml.hpp"

namespace opt = susml::optimized;
namespace min = susml::factory;

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransitionForCT(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  return opt::Transition(source, target, true, std::make_tuple(),
                         std::make_tuple([&] { counter += Index; }));
}

template <std::size_t... Indices>
constexpr auto makeTransitionsForCT(const std::index_sequence<Indices...> &,
                                    std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::make_tuple(
      makeTransitionForCT<Indices, totalTransitions>(counter)...);
}

template <std::size_t NumTransitions>
static void benchCTOptimizedCircularWithCounter(benchmark::State &s) {
  const auto numTriggers = s.range(0);

  std::size_t counter = 0;

  auto tuple =
      makeTransitionsForCT(std::make_index_sequence<NumTransitions>(), counter);

  auto m = opt::StateMachine<decltype(tuple)>{tuple, 0};

  for (auto _ : s) {
    for (int i = 0; i < numTriggers; i++) {
      m.trigger(true);
    }
  }

  s.counters["c"] = counter;
  s.counters["c/it"] =
      static_cast<double>(counter) / static_cast<double>(s.iterations());
}

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransitionForMin(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  return min::From(source)
      .To(target)
      .On(true)
      .Do({std::function([&] { counter += Index; })})
      .make();
}

template <std::size_t... Indices>
constexpr auto makeTransitionsForMin(const std::index_sequence<Indices...> &,
                                     std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::vector{
      makeTransitionForMin<Indices, totalTransitions>(counter)...};
}

template <std::size_t NumTransitions>
static void benchMinimalCircularWithCounter(benchmark::State &s) {
  const auto numTriggers = s.range(0);

  std::size_t counter = 0;

  std::vector transitions = makeTransitionsForMin(
      std::make_index_sequence<NumTransitions>(), counter);
  
  using TransitionContainer = decltype(transitions);
  using Transition = typename TransitionContainer::value_type;

  auto m = susml::StateMachine<Transition, TransitionContainer>{transitions, 0};

  for (auto _ : s) {
    for (int i = 0; i < numTriggers; i++) {
      m.trigger(true);
    }
  }

  s.counters["c"] = counter;
  s.counters["c/it"] =
      static_cast<double>(counter) / static_cast<double>(s.iterations());
}

BENCHMARK_TEMPLATE(benchCTOptimizedCircularWithCounter, 3)
    ->DenseRange(128, 1024, 128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(benchCTOptimizedCircularWithCounter, 10)
    ->DenseRange(128, 1024, 128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(benchCTOptimizedCircularWithCounter, 20)
    ->DenseRange(128, 1024, 128)->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(benchMinimalCircularWithCounter, 3)
    ->DenseRange(128, 1024, 128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(benchMinimalCircularWithCounter, 10)
    ->DenseRange(128, 1024, 128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(benchMinimalCircularWithCounter, 20)
    ->DenseRange(128, 1024, 128)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();