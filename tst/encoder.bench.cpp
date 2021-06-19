#include <benchmark/benchmark.h>
#include <random>
#include <stdexcept>

#include "factory.hpp"
#include "tuplebased.hpp"
#include "vectorbased.hpp"

namespace eventBased {
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
void trigger(eventBased::Event event, eventBased::State &currentState,
             int &delta) {
  using namespace eventBased;
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

static void encoderHCEventBased(benchmark::State &s) {
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

static void encoderVBEventBased(benchmark::State &s) {
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
  auto NoGuard = [] { return true; };
  auto NoAction = [] {};

  auto transitions = std::make_tuple(
      Transition(State::idle, State::clockwise1, Event::updateB, NoGuard,
                 NoAction),
      Transition(State::clockwise1, State::idle, Event::updateB, NoGuard,
                 NoAction),
      Transition(State::clockwise1, State::clockwise2, Event::updateA, NoGuard,
                 NoAction),
      Transition(State::clockwise2, State::clockwise1, Event::updateA, NoGuard,
                 NoAction),
      Transition(State::clockwise2, State::clockwise3, Event::updateB, NoGuard,
                 NoAction),
      Transition(State::clockwise3, State::clockwise2, Event::updateB, NoGuard,
                 NoAction),
      Transition(State::clockwise3, State::idle, Event::updateA, NoGuard,
                 [&] { delta++; }),
      Transition(State::idle, State::counterclockwise1, Event::updateA, NoGuard,
                 NoAction),
      Transition(State::counterclockwise1, State::idle, Event::updateA, NoGuard,
                 NoAction),
      Transition(State::counterclockwise1, State::counterclockwise2,
                 Event::updateB, NoGuard, NoAction),
      Transition(State::counterclockwise2, State::counterclockwise1,
                 Event::updateB, NoGuard, NoAction),
      Transition(State::counterclockwise2, State::counterclockwise3,
                 Event::updateA, NoGuard, NoAction),
      Transition(State::counterclockwise3, State::counterclockwise2,
                 Event::updateA, NoGuard, NoAction),
      Transition(State::counterclockwise3, State::idle, Event::updateB, NoGuard,
                 [&] { delta--; }));

  return susml::tuplebased::StateMachine<State, Event, decltype(transitions)>(
      State::idle, transitions);
}

static void encoderTBEventBased(benchmark::State &s) {
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
} // namespace eventBased

namespace guardBased {
enum class State {
  idle,
  clockwise1,
  clockwise2,
  clockwise3,
  counterclockwise1,
  counterclockwise2,
  counterclockwise3
};

enum class Event { update };

struct Update {
  bool newA = false;
  bool newB = false;
};

namespace handcrafted {
void trigger(guardBased::State &currentState, int &delta, bool &a, bool &b) {
  using namespace guardBased;
  switch (currentState) {
  case State::idle: { // false false
    if (a && !b) {
      currentState = State::counterclockwise1;
    } else if (!a && b) {
      currentState = State::clockwise1;
    }
    break;
  }
  case State::clockwise1: { // false true
    if (a && b) {
      currentState = State::clockwise2;
    } else if (!a && !b) {
      currentState = State::idle;
    }
    break;
  }
  case State::clockwise2: { // true true
    if (a && !b) {
      currentState = State::clockwise1;
    } else if (!a && b) {
      currentState = State::clockwise3;
    }
    break;
  }
  case State::clockwise3: { // true false
    if (!a && !b) {
      delta++;
      currentState = State::idle;
    } else if (a && b) {
      currentState = State::clockwise2;
    }
    break;
  }
  case State::counterclockwise1: { // true false
    if (a && b) {
      currentState = State::idle;
    } else if (!a && !b) {
      currentState = State::counterclockwise2;
    }
    break;
  }
  case State::counterclockwise2: { // true true
    if (!a && b) {
      currentState = State::counterclockwise3;
    } else if (!b && a) {
      currentState = State::counterclockwise1;
    }
    break;
  }
  case State::counterclockwise3: { // false true
    if (a && b) {
      currentState = State::counterclockwise2;
    } else if (!a && !b) {
      delta--;
      currentState = State::idle;
    }
    break;
  }
  }
}

static void encoderHCGuardBased(benchmark::State &s) {
  int delta = 0;
  bool a = false;
  bool b = false;
  State currentState = State::idle;

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, 1);

  auto getUpdates = [&] {
    std::vector<Update> updates(s.range(0));

    updates[0].newA = false;
    updates[0].newA = false;

    for (std::size_t i = 1; i < updates.size(); i++) {
      updates[i] = updates[i - 1];

      const auto r = dist(mt);
      if (r == 0) {
        updates[i].newA = !updates[i - 1].newA;
      } else {
        updates[i].newB = !updates[i - 1].newB;
      }
    }

    return updates;
  };

  for (auto _ : s) {
    s.PauseTiming();
    auto updates = getUpdates();
    s.ResumeTiming();

    for (const auto u : updates) {
      a = u.newA;
      b = u.newB;

      trigger(currentState, delta, a, b);
    }
  }

  s.counters["d"] = delta;
}
} // namespace handcrafted

namespace vectorbased {

auto makeStateMachine(int &delta, const bool &a, const bool &b) {
  using namespace susml::factory;
  using susml::vectorbased::StateMachine;

  auto Fn = [](auto e) { return std::function(e); };
  auto And = [&](bool desiredA, bool desiredB) {
    return Fn([&] { return (a == desiredA && b == desiredB); });
  };
  auto NoAction = Fn([] {});

  std::vector transitions = {From(State::idle) // false false
                                 .To(State::clockwise1)
                                 .On(Event::update)
                                 .If(And(false, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise1) // false true
                                 .To(State::idle)
                                 .On(Event::update)
                                 .If(And(false, false))
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise1) // false true
                                 .To(State::clockwise2)
                                 .On(Event::update)
                                 .If(And(true, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise2) // true true
                                 .To(State::clockwise1)
                                 .On(Event::update)
                                 .If(And(false, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise2) // true true
                                 .To(State::clockwise3)
                                 .On(Event::update)
                                 .If(And(true, false))
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise3) // true false
                                 .To(State::clockwise2)
                                 .On(Event::update)
                                 .If(And(true, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::clockwise3) // true false
                                 .To(State::idle)
                                 .On(Event::update)
                                 .If(And(false, false))
                                 .Do(Fn([&] { delta++; }))
                                 .make(),
                             From(State::idle) // false false
                                 .To(State::counterclockwise1)
                                 .On(Event::update)
                                 .If(And(true, false))
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise1) // true false
                                 .To(State::idle)
                                 .On(Event::update)
                                 .If(And(false, false))
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise1) // true false
                                 .To(State::counterclockwise2)
                                 .On(Event::update)
                                 .If(And(true, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise2) // true true
                                 .To(State::counterclockwise1)
                                 .On(Event::update)
                                 .If(And(true, false))
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise2) // true true
                                 .To(State::counterclockwise3)
                                 .On(Event::update)
                                 .If(And(false, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise3) // false true
                                 .To(State::counterclockwise2)
                                 .On(Event::update)
                                 .If(And(true, true))
                                 .Do(NoAction)
                                 .make(),
                             From(State::counterclockwise3) // false true
                                 .To(State::idle)
                                 .On(Event::update)
                                 .If(And(false, false))
                                 .Do(Fn([&] { delta--; }))
                                 .make()};

  return StateMachine<decltype(transitions)::value_type>{State::idle,
                                                         transitions};
}

static void encoderVBGuardBased(benchmark::State &s) {
  int delta = 0;
  bool a = false;
  bool b = false;
  auto m = makeStateMachine(delta, a, b);

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, 1);

  auto getUpdates = [&] {
    std::vector<Update> updates(s.range(0));

    updates[0].newA = false;
    updates[0].newA = false;

    for (std::size_t i = 1; i < updates.size(); i++) {
      updates[i] = updates[i - 1];

      const auto r = dist(mt);
      if (r == 0) {
        updates[i].newA = !updates[i - 1].newA;
      } else {
        updates[i].newB = !updates[i - 1].newB;
      }
    }

    return updates;
  };

  for (auto _ : s) {
    s.PauseTiming();
    auto updates = getUpdates();
    s.ResumeTiming();

    for (const Update &u : updates) {
      a = u.newA;
      b = u.newB;

      m.trigger(Event::update);
    }
  }

  s.counters["d"] = delta;
}

} // namespace vectorbased

namespace tuplebased {
using susml::Transition;

auto makeStateMachine(int &delta, bool &a, bool &b) {
  auto And = [&](bool desiredA, bool desiredB) {
    return
        [&a, &b, desiredA, desiredB] { return a == desiredA && b == desiredB; };
  };
  auto NoAction = [&] {};

  auto transitions = std::make_tuple(
      Transition(State::idle, // false false
                 State::clockwise1, Event::update, And(false, true), NoAction),
      Transition(State::clockwise1, // false true
                 State::idle, Event::update, And(false, false), NoAction),
      Transition(State::clockwise1, // false true
                 State::clockwise2, Event::update, And(true, true), NoAction),
      Transition(State::clockwise2, // true true
                 State::clockwise1, Event::update, And(false, true), NoAction),
      Transition(State::clockwise2, // true true
                 State::clockwise3, Event::update, And(true, false), NoAction),
      Transition(State::clockwise3, // true false
                 State::clockwise2, Event::update, And(true, true), NoAction),
      Transition(State::clockwise3, // true false
                 State::idle, Event::update, And(false, false),
                 [&] { delta++; }),
      Transition(State::idle, // false false
                 State::counterclockwise1, Event::update, And(true, false),
                 NoAction),
      Transition(State::counterclockwise1, // true false
                 State::idle, Event::update, And(false, false), NoAction),
      Transition(State::counterclockwise1, // true false
                 State::counterclockwise2, Event::update, And(true, true),
                 NoAction),
      Transition(State::counterclockwise2, // true true
                 State::counterclockwise1, Event::update, And(true, false),
                 NoAction),
      Transition(State::counterclockwise2, // true true
                 State::counterclockwise3, Event::update, And(false, true),
                 NoAction),
      Transition(State::counterclockwise3, // false true
                 State::counterclockwise2, Event::update, And(true, true),
                 NoAction),
      Transition(State::counterclockwise3, // false true
                 State::idle, Event::update, And(false, false),
                 [&] { delta--; }));

  return susml::tuplebased::StateMachine<State, Event, decltype(transitions)>(
      State::idle, transitions);
}

static void encoderTBGuardBased(benchmark::State &s) {
  int delta = 0;
  bool a = false;
  bool b = false;
  auto m = makeStateMachine(delta, a, b);

  static std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<short> dist(0, 1);

  auto getUpdates = [&] {
    std::vector<Update> updates(s.range(0));

    updates[0].newA = false;
    updates[0].newA = false;

    for (std::size_t i = 1; i < updates.size(); i++) {
      updates[i] = updates[i - 1];

      const auto r = dist(mt);
      if (r == 0) {
        updates[i].newA = !updates[i - 1].newA;
      } else {
        updates[i].newB = !updates[i - 1].newB;
      }
    }

    return updates;
  };

  for (auto _ : s) {
    s.PauseTiming();
    auto updates = getUpdates();
    s.ResumeTiming();

    for (const Update &u : updates) {
      a = u.newA;
      b = u.newB;

      m.trigger(Event::update);
    }
  }

  s.counters["d"] = delta;
}

} // namespace tuplebased
} // namespace guardBased

using eventBased::handcrafted::encoderHCEventBased;
using eventBased::tuplebased::encoderTBEventBased;
using eventBased::vectorbased::encoderVBEventBased;
using guardBased::handcrafted::encoderHCGuardBased;
using guardBased::tuplebased::encoderTBGuardBased;
using guardBased::vectorbased::encoderVBGuardBased;

constexpr auto numTriggersLowerBound = (1 << 15);
constexpr auto numTriggersUpperBound = (1 << 20);

BENCHMARK(encoderTBGuardBased)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(encoderVBGuardBased)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(encoderTBEventBased)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(encoderVBEventBased)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(encoderHCEventBased)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(encoderHCGuardBased)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();