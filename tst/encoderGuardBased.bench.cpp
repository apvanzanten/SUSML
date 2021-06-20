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
  counterclockwise3
};

enum class Event { update };

struct Update {
  bool newA = false;
  bool newB = false;
};

namespace handcrafted {
void trigger(State &currentState, int &delta, bool &a, bool &b) {
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

static void encoderGuardBasedHC(benchmark::State &s) {
  int   delta        = 0;
  bool  a            = false;
  bool  b            = false;
  State currentState = State::idle;

  static std::mt19937                  mt{std::random_device{}()};
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

  auto Fn  = [](auto e) { return std::function(e); };
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

  return StateMachine<decltype(transitions)::value_type>{State::idle, transitions};
}

static void encoderGuardBasedVB(benchmark::State &s) {
  int  delta = 0;
  bool a     = false;
  bool b     = false;
  auto m     = makeStateMachine(delta, a, b);

  static std::mt19937                  mt{std::random_device{}()};
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
    return [&a, &b, desiredA, desiredB] { return a == desiredA && b == desiredB; };
  };
  auto transitions = std::make_tuple(Transition(State::idle, // false false
                                                State::clockwise1,
                                                Event::update,
                                                And(false, true)),
                                     Transition(State::clockwise1, // false true
                                                State::idle,
                                                Event::update,
                                                And(false, false)),
                                     Transition(State::clockwise1, // false true
                                                State::clockwise2,
                                                Event::update,
                                                And(true, true)),
                                     Transition(State::clockwise2, // true true
                                                State::clockwise1,
                                                Event::update,
                                                And(false, true)),
                                     Transition(State::clockwise2, // true true
                                                State::clockwise3,
                                                Event::update,
                                                And(true, false)),
                                     Transition(State::clockwise3, // true false
                                                State::clockwise2,
                                                Event::update,
                                                And(true, true)),
                                     Transition(State::clockwise3, // true false
                                                State::idle,
                                                Event::update,
                                                And(false, false),
                                                [&] { delta++; }),
                                     Transition(State::idle, // false false
                                                State::counterclockwise1,
                                                Event::update,
                                                And(true, false)),
                                     Transition(State::counterclockwise1, // true false
                                                State::idle,
                                                Event::update,
                                                And(false, false)),
                                     Transition(State::counterclockwise1, // true false
                                                State::counterclockwise2,
                                                Event::update,
                                                And(true, true)),
                                     Transition(State::counterclockwise2, // true true
                                                State::counterclockwise1,
                                                Event::update,
                                                And(true, false)),
                                     Transition(State::counterclockwise2, // true true
                                                State::counterclockwise3,
                                                Event::update,
                                                And(false, true)),
                                     Transition(State::counterclockwise3, // false true
                                                State::counterclockwise2,
                                                Event::update,
                                                And(true, true)),
                                     Transition(State::counterclockwise3, // false true
                                                State::idle,
                                                Event::update,
                                                And(false, false),
                                                [&] { delta--; }));

  return susml::tuplebased::StateMachine<State, Event, decltype(transitions)>(State::idle,
                                                                              transitions);
}

static void encoderGuardBasedTB(benchmark::State &s) {
  int  delta = 0;
  bool a     = false;
  bool b     = false;
  auto m     = makeStateMachine(delta, a, b);

  static std::mt19937                  mt{std::random_device{}()};
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

using handcrafted::encoderGuardBasedHC;
using tuplebased::encoderGuardBasedTB;
using vectorbased::encoderGuardBasedVB;

constexpr auto numTriggersLowerBound = (1 << 15);
constexpr auto numTriggersUpperBound = (1 << 20);

BENCHMARK(encoderGuardBasedHC)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK(encoderGuardBasedTB)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK(encoderGuardBasedVB)
    ->RangeMultiplier(2)
    ->Range(numTriggersLowerBound, numTriggersUpperBound)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();