#include <benchmark/benchmark.h>
#include <tuple>
#include <type_traits>
#include <utility>

#include "factory.hpp"
#include "susml.hpp"
#include "tuplebased.hpp"

namespace tuplebased {
using susml::tuplebased::StateMachine;
using susml::tuplebased::Transition;

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr std::size_t source = Index;
  constexpr std::size_t target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

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
static void benchTupleBased(benchmark::State &s) {
  const auto numTriggers = s.range(0);

  std::size_t counter = 0;

  auto tuple =
      makeTransitions(std::make_index_sequence<NumTransitions>(), counter);

  auto m = StateMachine<std::size_t, bool, decltype(tuple)>(tuple, 0);

  for (auto _ : s) {
    for (int i = 0; i < numTriggers; i++) {
      m.trigger(true);
    }
  }

  s.counters["c"] = counter;
  s.counters["c/it"] =
      static_cast<double>(counter) / static_cast<double>(s.iterations());
}
} // namespace tuplebased

namespace minimal {
using susml::StateMachine;
using namespace susml::factory;

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
static void benchMinimal(benchmark::State &s) {
  const auto numTriggers = s.range(0);

  std::size_t counter = 0;

  std::vector transitions =
      makeTransitions(std::make_index_sequence<NumTransitions>(), counter);

  using TransitionContainer = decltype(transitions);
  using Transition = typename TransitionContainer::value_type;

  auto m = StateMachine<Transition, TransitionContainer>{transitions, 0};

  for (auto _ : s) {
    for (int i = 0; i < numTriggers; i++) {
      m.trigger(true);
    }
  }

  s.counters["c"] = counter;
  s.counters["c/it"] =
      static_cast<double>(counter) / static_cast<double>(s.iterations());
}

} // namespace minimal

#define BENCH(NumTransitions)                                                  \
  namespace {                                                                  \
    using tuplebased::benchTupleBased;                                         \
    using minimal::benchMinimal;                                               \
    BENCHMARK_TEMPLATE(benchTupleBased, NumTransitions)                        \
        ->Arg(10000)                                                           \
        ->Unit(benchmark::kMicrosecond);                                       \
    BENCHMARK_TEMPLATE(benchMinimal, NumTransitions)                           \
        ->Arg(10000)                                                           \
        ->Unit(benchmark::kMicrosecond);                                       \
  }

BENCH(2);
BENCH(4);
BENCH(5);
BENCH(6);
BENCH(7);
BENCH(8);
BENCH(16);

BENCHMARK_MAIN();