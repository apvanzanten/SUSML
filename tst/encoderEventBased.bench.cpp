#include <benchmark/benchmark.h>
#include <random>

#include "factory.hpp"
#include "tuplebased.hpp"
#include "vectorbased.hpp"

enum class State {
  idle,
  clockwise1,
  clockwise2,
  clockwise3,
  counterclockwise1,
  counterclockwise2,
  counterclockwise3,
};

enum class Event { updateA, updateB };

namespace handcrafted {
void trigger(Event event, State &currentState, int &delta) {
  switch (currentState) {
  case State::idle: {
    if (event == Event::updateA) {
      currentState = State::counterclockwise1;
    } else if (event == Event::updateB) {
      currentState = State::clockwise1;
    }
    break;
  }
  case State::clockwise1: {
    if (event == Event::updateA) {
      currentState = State::clockwise2;
    } else if (event == Event::updateB) {
      currentState = State::idle;
    }
    break;
  }
  case State::clockwise2: {
    if (event == Event::updateA) {
      currentState = State::clockwise1;
    } else if (event == Event::updateB) {
      currentState = State::clockwise3;
    }
    break;
  }
  case State::clockwise3: {
    if (event == Event::updateA) {
      delta++;
      currentState = State::idle;
    } else if (event == Event::updateB) {
      currentState = State::clockwise2;
    }
    break;
  }
  case State::counterclockwise1: {
    if (event == Event::updateA) {
      currentState = State::idle;
    } else if (event == Event::updateB) {
      currentState = State::counterclockwise2;
    }
    break;
  }
  case State::counterclockwise2: {
    if (event == Event::updateA) {
      currentState = State::counterclockwise3;
    } else if (event == Event::updateB) {
      currentState = State::counterclockwise1;
    }
    break;
  }
  case State::counterclockwise3: {
    if (event == Event::updateA) {
      currentState = State::counterclockwise2;
    } else if (event == Event::updateB) {
      delta--;
      currentState = State::idle;
    }
    break;
  }
  }
}

static void encoderEventBasedHC(benchmark::State &s) {
  int delta = 0;
  State currentState = State::idle;

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, 1);

  auto getEvents = [&] {
    std::vector<Event> events(s.range(0));
    for (auto &e : events) {
      e = (dist(mt) == 0) ? Event::updateA : Event::updateB;
    }
    return events;
  };

  for (auto _ : s) {
    s.PauseTiming();
    auto events = getEvents();
    s.ResumeTiming();

    for (const Event e : events) {
      trigger(e, currentState, delta);
    }
  }

  s.counters["d"] = delta;
}
} // namespace handcrafted

namespace vectorbased {

auto makeStateMachine(int &delta) {
  using namespace susml::factory;
  using susml::vectorbased::StateMachine;

  auto Fn = [](auto e) { return std::function(e); };
  auto NoAction = Fn([] {});

  std::vector transitions = {From(State::idle)
                                 .To(State::clockwise1)
                                 .On(Event::updateB)
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise1)
                                 .To(State::idle)
                                 .On(Event::updateB)
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise1)
                                 .To(State::clockwise2)
                                 .On(Event::updateA)
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise2)
                                 .To(State::clockwise1)
                                 .On(Event::updateA)
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise2)
                                 .To(State::clockwise3)
                                 .On(Event::updateB)
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise3)
                                 .To(State::clockwise2)
                                 .On(Event::updateB)
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise3)
                                 .To(State::idle)
                                 .On(Event::updateA)
                                 .Do(Fn([&] { delta++; }))
                                 .make(),
                             From(State::idle)
                                 .To(State::counterclockwise1)
                                 .On(Event::updateA)
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise1)
                                 .To(State::idle)
                                 .On(Event::updateA)
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise1)
                                 .To(State::counterclockwise2)
                                 .On(Event::updateB)
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise2)
                                 .To(State::counterclockwise1)
                                 .On(Event::updateB)
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise2)
                                 .To(State::counterclockwise3)
                                 .On(Event::updateA)
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise3)
                                 .To(State::counterclockwise2)
                                 .On(Event::updateA)
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise3)
                                 .To(State::idle)
                                 .On(Event::updateB)
                                 .Do(Fn([&] { delta--; }))
                                 .make()};

  return StateMachine<decltype(transitions)::value_type>{State::idle,
                                                         transitions};
}

static void encoderEventBasedVB(benchmark::State &s) {
  int delta = 0;
  auto m = makeStateMachine(delta);

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, 1);

  auto getEvents = [&] {
    std::vector<Event> events(s.range(0));
    for (auto &e : events) {
      e = (dist(mt) == 0) ? Event::updateA : Event::updateB;
    }
    return events;
  };

  for (auto _ : s) {
    s.PauseTiming();
    auto events = getEvents();
    s.ResumeTiming();

    for (const Event e : events) {
      m.trigger(e);
    }
  }

  s.counters["d"] = delta;
}

} // namespace vectorbased

namespace tuplebased {
using susml::Transition;

auto makeStateMachine(int &delta) {
  auto NoGuard = susml::NoneType{};

  auto transitions = std::make_tuple(
      Transition(State::idle, State::clockwise1, Event::updateB),
      Transition(State::clockwise1, State::idle, Event::updateB),
      Transition(State::clockwise1, State::clockwise2, Event::updateA),
      Transition(State::clockwise2, State::clockwise1, Event::updateA),
      Transition(State::clockwise2, State::clockwise3, Event::updateB),
      Transition(State::clockwise3, State::clockwise2, Event::updateB),
      Transition(State::clockwise3, State::idle, Event::updateA, NoGuard,
                 [&] { delta++; }),
      Transition(State::idle, State::counterclockwise1, Event::updateA),
      Transition(State::counterclockwise1, State::idle, Event::updateA),
      Transition(State::counterclockwise1, State::counterclockwise2,
                 Event::updateB),
      Transition(State::counterclockwise2, State::counterclockwise1,
                 Event::updateB),
      Transition(State::counterclockwise2, State::counterclockwise3,
                 Event::updateA),
      Transition(State::counterclockwise3, State::counterclockwise2,
                 Event::updateA),
      Transition(State::counterclockwise3, State::idle, Event::updateB, NoGuard,
                 [&] { delta--; }));

  return susml::tuplebased::StateMachine<State, Event, decltype(transitions)>(
      State::idle, transitions);
}

static void encoderEventBasedTB(benchmark::State &s) {
  int delta = 0;
  auto m = makeStateMachine(delta);

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, 1);

  auto getEvents = [&] {
    std::vector<Event> events(s.range(0));
    for (auto &e : events) {
      e = (dist(mt) == 0) ? Event::updateA : Event::updateB;
    }
    return events;
  };

  for (auto _ : s) {
    s.PauseTiming();
    auto events = getEvents();
    s.ResumeTiming();

    for (const Event &e : events) {
      m.trigger(e);
    }
  }

  s.counters["d"] = delta;
}
} // namespace tuplebased

using handcrafted::encoderEventBasedHC;
using tuplebased::encoderEventBasedTB;
using vectorbased::encoderEventBasedVB;

constexpr auto numTriggersLowerBound = (1 << 15);
constexpr auto numTriggersUpperBound = (1 << 20);

BENCHMARK(encoderEventBasedHC)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK(encoderEventBasedTB)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK(encoderEventBasedVB)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();