#include <benchmark/benchmark.h>
#include <cstddef>
#include <random>
#include <stdexcept>

#include "dataoriented/StateMachine.hpp"
#include "factory.hpp"
#include "minimal/StateMachine.hpp"

using State = std::size_t;
using Event = std::size_t;
using Guard = std::function<bool()>;
using Action = std::function<void()>;

using Transition = susml::Transition<State, Event, Guard, Action>;

std::vector<Transition> makeTransitionsForEvent(std::vector<State> states,
                                                Event event,
                                                std::size_t &counter) {
  std::vector<Transition> v;
  v.reserve(states.size() + 1);

  static std::mt19937 mt{std::random_device{}()};
  std::shuffle(states.begin(), states.end(), mt);

  for (std::size_t i = 1; i < states.size(); i++) {
    v.emplace_back(states[i - 1], states[i], event, Guard{[] { return true; }},
                   Action{[&counter] { counter++; }});
  }

  v.emplace_back(states.back(), states.front(), event,
                 Guard{[] { return true; }}, Action{[&counter] { counter++; }});

  return v;
}

std::vector<Transition> makeTransitions(std::size_t numStates,
                                        std::size_t numEvents,
                                        std::size_t &counter) {
  std::vector<State> states(numStates, 0);
  std::generate(states.begin(), states.end(),
                [n = 0]() mutable { return n++; });

  std::vector<Event> events(numEvents, 0);
  std::generate(events.begin(), events.end(),
                [n = 0]() mutable { return n++; });

  std::vector<Transition> transitions;

  for (auto e : events) {
    auto newTransitions = makeTransitionsForEvent(states, e, counter);
    transitions.insert(transitions.end(), newTransitions.begin(),
                       newTransitions.end());
  }

  return transitions;
}

std::vector<Event> makeTriggers(std::size_t numEvents,
                                std::size_t numTriggers) {
  std::vector<Event> triggers(numTriggers, 0);

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, numEvents - 1);

  std::generate(triggers.begin(), triggers.end(), [&]() { return dist(mt); });

  return triggers;
}

namespace minimal {
using StateMachine =
    susml::minimal::StateMachine<Transition, std::vector<Transition>>;

StateMachine makeStateMachine(std::size_t numStates, std::size_t numEvents,
                              std::size_t &counter) {
  return {makeTransitions(numStates, numEvents, counter), 0};
}

static void bigMachineMinimal(benchmark::State &s) {
  const auto numStates = s.range(0);
  const auto numEvents = s.range(1);
  const auto numTriggers = s.range(2);

  std::size_t counter = 0;

  auto m = makeStateMachine(numStates, numEvents, counter);

  for (auto _ : s) {
    s.PauseTiming();
    auto triggers = makeTriggers(numEvents, numTriggers);
    s.ResumeTiming();

    for (Event t : triggers) {
      m.trigger(t);
    }
  }

  s.counters["c"] = counter;
  s.counters["#Transitions"] = m.transitions.size();
  s.counters["#Bytes"] = sizeof(Transition) * m.transitions.size();
}

} // namespace minimal

namespace dataoriented {
using StateMachine =
    susml::dataoriented::StateMachine<State, Event, Guard, Action>;
using susml::dataoriented::fromTransitions;

StateMachine makeStateMachine(std::size_t numStates, std::size_t numEvents,
                              std::size_t &counter) {
  return fromTransitions(State{0},
                         makeTransitions(numStates, numEvents, counter));
}

static void bigMachineDataOriented(benchmark::State &s) {
  const auto numStates = s.range(0);
  const auto numEvents = s.range(1);
  const auto numTriggers = s.range(2);

  std::size_t counter = 0;

  auto m = makeStateMachine(numStates, numEvents, counter);

  for (auto _ : s) {
    s.PauseTiming();
    auto triggers = makeTriggers(numEvents, numTriggers);
    s.ResumeTiming();

    for (Event t : triggers) {
      m.trigger(t);
    }
  }

  s.counters["c"] = counter;
  s.counters["#Transitions"] = m.sources.size();
  s.counters["#Bytes"] = (sizeof(m) + ((sizeof(State) * 2) + sizeof(Event) +
                                       sizeof(Guard) + sizeof(Action)) *
                                          m.sources.size());
}
} // namespace dataoriented

using dataoriented::bigMachineDataOriented;
using minimal::bigMachineMinimal;

static void CustomArguments(benchmark::internal::Benchmark *b) {
  for (int numStates = (1 << 10); numStates < (1 << 15); numStates *= 2) {
    for (int numEvents = (1 << 4); numEvents < (1 << 11); numEvents *= 2) {
      b->Args({numStates, numEvents, 1000});
    }
  }
}

BENCHMARK(bigMachineMinimal)
    ->Apply(CustomArguments)
    ->Unit(benchmark::kMillisecond);
BENCHMARK(bigMachineDataOriented)
    ->Apply(CustomArguments)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();