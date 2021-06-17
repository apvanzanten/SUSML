#include <benchmark/benchmark.h>
#include <random>
#include <stdexcept>

#include "minimal/StateMachine.hpp"
#include "minimal/factory.hpp"
#include "tuplebased/StateMachine.hpp"

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

namespace minimal {

auto makeStateMachine(int &delta) {
  using namespace susml::minimal::factory;
  using susml::minimal::StateMachine;

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

  return StateMachine<decltype(transitions)::value_type, decltype(transitions)>{
      transitions, State::idle};
}

static void benchMEventBased(benchmark::State &s) {
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

} // namespace minimal
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

namespace minimal {

auto makeStateMachine(int &delta, const bool &a, const bool &b) {
  using namespace susml::minimal::factory;
  using susml::minimal::StateMachine;

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

  return StateMachine<decltype(transitions)::value_type, decltype(transitions)>{
      transitions, State::idle};
}

template <typename... Args>
void testMachine(susml::minimal::StateMachine<Args...> &m, int &delta, bool &a,
                 bool &b) {
  const bool aStart = a;
  const bool bStart = b;
  const int deltaStart = delta;
  const State currentStateStart = m.currentState;

  a = false;
  b = false;
  delta = 0;
  m.currentState = State::idle;

  auto makeUpdate = [&](Update update) {
    a = update.newA;
    b = update.newB;
    m.trigger(Event::update);
  };

  auto trigger = [&](auto... updates) { (..., makeUpdate(updates)); };

  // full clockwise
  trigger(Update{false, true}, Update{true, true}, Update{true, false},
          Update{false, false});

  if (!(m.currentState == State::idle && delta == 1)) {
    throw std::runtime_error("machine failed at full clockwise");
  }

  // full counterclockwise
  trigger(Update{true, false}, Update{true, true}, Update{false, true},
          Update{false, false});

  if (!(m.currentState == State::idle && delta == 0)) {
    throw std::runtime_error("machine failed at full counterclockwise");
  }

  // halfway clockwise
  trigger(Update{false, true}, // cw1
          Update{true, true},  // cw2
          Update{true, false}, // cw3
          Update{true, true},  // cw2
          Update{false, true}, // cw1
          Update{false, false} // idle
  );

  if (!(m.currentState == State::idle && delta == 0)) {
    throw std::runtime_error("machine failed at halfway clockwise");
  }

  // halfway counterclockwise
  trigger(Update{true, false}, // ccw1
          Update{true, true},  // ccw2
          Update{false, true}, // ccw3
          Update{true, true},  // ccw2
          Update{true, false}, // ccw1
          Update{false, false} // idle
  );

  if (!(m.currentState == State::idle && delta == 0)) {
    throw std::runtime_error("machine failed at halfway counterclockwise");
  }

  // error clockwise
  trigger(Update{false, true}, // cw1
          Update{true, true},  // cw2
          Update{true, false}, // cw3
          Update{false, true}, // error
          Update{false, false} // idle
  );

  a = aStart;
  b = bStart;
  delta = deltaStart;
  m.currentState = currentStateStart;
}

static void benchMGuardBased(benchmark::State &s) {
  int delta = 0;
  bool a = false;
  bool b = false;
  auto m = makeStateMachine(delta, a, b);

  testMachine(m, delta, a, b);

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

} // namespace minimal

namespace tuplebased {
using susml::tuplebased::Transition;

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
      transitions, State::idle);
}

template <typename... Args>
void testMachine(susml::tuplebased::StateMachine<Args...> &m, int &delta,
                 bool &a, bool &b) {
  const bool aStart = a;
  const bool bStart = b;
  const int deltaStart = delta;
  const State currentStateStart = m.currentState();

  a = false;
  b = false;
  delta = 0;
  m.setState(State::idle);

  auto makeUpdate = [&](Update update) {
    a = update.newA;
    b = update.newB;
    m.trigger(Event::update);
  };

  auto trigger = [&](auto... updates) { (..., makeUpdate(updates)); };

  // full clockwise
  trigger(Update{false, true}, // cw1
          Update{true, true},  // cw2
          Update{true, false}, // cw3
          Update{false, false} // idle
  );

  if (!(m.currentState() == State::idle && delta == 1)) {
    throw std::runtime_error("machine failed at full clockwise");
  }

  // full counterclockwise
  trigger(Update{true, false}, Update{true, true}, Update{false, true},
          Update{false, false});

  if (!(m.currentState() == State::idle && delta == 0)) {
    throw std::runtime_error("machine failed at full counterclockwise");
  }

  // halfway clockwise
  trigger(Update{false, true}, // cw1
          Update{true, true},  // cw2
          Update{true, false}, // cw3
          Update{true, true},  // cw2
          Update{false, true}, // cw1
          Update{false, false} // idle
  );

  if (!(m.currentState() == State::idle && delta == 0)) {
    throw std::runtime_error("machine failed at halfway clockwise");
  }

  // halfway counterclockwise
  trigger(Update{true, false}, // ccw1
          Update{true, true},  // ccw2
          Update{false, true}, // ccw3
          Update{true, true},  // ccw2
          Update{true, false}, // ccw1
          Update{false, false} // idle
  );

  if (!(m.currentState() == State::idle && delta == 0)) {
    throw std::runtime_error("machine failed at halfway counterclockwise");
  }

  a = aStart;
  b = bStart;
  delta = deltaStart;
  m.setState(currentStateStart);
}

static void benchTBGuardBased(benchmark::State &s) {
  int delta = 0;
  bool a = false;
  bool b = false;
  auto m = makeStateMachine(delta, a, b);

  testMachine(m, delta, a, b);

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

using eventBased::minimal::benchMEventBased;
using guardBased::minimal::benchMGuardBased;
using guardBased::tuplebased::benchTBGuardBased;

BENCHMARK(benchTBGuardBased)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(benchMGuardBased)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(benchMEventBased)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();