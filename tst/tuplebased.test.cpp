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

#include "gtest/gtest.h"

#include "tuplebased/StateMachine.hpp"

bool trueGuard() { return true; }

void noneAction() {}

TEST(ValidationTests, transitions) {
  using susml::Transition;
  using susml::tuplebased::validate::isTransitionType;

  enum class State { a, b, c };
  enum class Event { x, y, z };

  auto someGuard = [] { return true; };
  auto someAction = [] {};

  EXPECT_TRUE(
      (isTransitionType<Transition<int, int, std::tuple<>, std::tuple<>>>()));
  EXPECT_TRUE((
      isTransitionType<Transition<State, Event, std::tuple<decltype(someGuard)>,
                                  std::tuple<decltype(someAction)>>>()));

  EXPECT_FALSE(isTransitionType<int>());
}

TEST(ValidationTests, transitionTuples) {
  using susml::Transition;
  using susml::tuplebased::validate::isValidTransitionTupleType;

  EXPECT_TRUE((isValidTransitionTupleType<
               std::tuple<Transition<int, bool, bool (*)(), void (*)()>,
                          Transition<int, bool, bool (*)(), void (*)()>>,
               int, bool>()));

  EXPECT_FALSE((isValidTransitionTupleType<
                std::tuple<Transition<int, int, bool (*)(), void (*)()>,
                           Transition<int, bool, bool (*)(), void (*)()>>,
                int, bool>()));

  EXPECT_FALSE((isValidTransitionTupleType<
                std::tuple<Transition<int, bool, bool (*)(), void (*)()>,
                           Transition<int, int, bool (*)(), void (*)()>>,
                int, bool>()));

  EXPECT_TRUE(
      (isValidTransitionTupleType<
          std::tuple<Transition<int, bool, std::function<bool()>, void (*)()>,
                     Transition<int, bool, bool (*)(), void (*)()>>,
          int, bool>()));

  EXPECT_FALSE((isValidTransitionTupleType<
                std::tuple<Transition<int, int, bool (*)(), void (*)()>,
                           Transition<int, bool, bool (*)(), void (*)()>>,
                int, bool>()));

  EXPECT_FALSE((isValidTransitionTupleType<
                std::tuple<Transition<int, bool, bool (*)(), void (*)()>,
                           Transition<int, int, bool (*)(), void (*)()>>,
                int, int>()));
}

TEST(TransitionTests, basic) {
  using susml::Transition;

  enum class State { off, on };
  enum class Event { turnOn, turnOff };

  const auto t = Transition{State::off, State::on, Event::turnOn,
                            [] { return true; }, [] {}};

  EXPECT_EQ(State::off, t.source);
  EXPECT_EQ(State::on, t.target);
  EXPECT_EQ(Event::turnOn, t.event);

  EXPECT_TRUE(t.guard());
}

TEST(TransitionTests, basicWithGuard) {
  using susml::Transition;

  bool val = false;
  auto getVal = [&val] { return val; };

  auto t = Transition{0, 0, 0, // 0 integers because we don't really care about
                               // states and events for this test
                      getVal, [] {}};

  EXPECT_FALSE(t.guard()) << "should return false because val is false";

  val = true;
  EXPECT_TRUE(t.guard()) << "should return true because val is true";
}

TEST(TransitionTests, basicWithAction) {
  using susml::Transition;

  bool val = false;
  auto unitGuard = [] { return true; };
  auto setVal = [&val] { val = true; };

  auto t = Transition(0, 0, 0, // 0 integers because we don't really care about
                               // states and events for this test
                      unitGuard, setVal);

  ASSERT_FALSE(val);

  t.action(); // should set val to true
  EXPECT_TRUE(val);
}

TEST(StateMachineTests, basicTransition) {

  enum class State { on, off };
  enum class Event { turnOn, turnOff };

  using Transition = susml::Transition<State, Event, bool (*)(), void (*)()>;
  using StateMachine =
      susml::tuplebased::StateMachine<State, Event,
                                      std::tuple<Transition, Transition>>;

  Transition onToOff{State::on, State::off, Event::turnOff, [] { return true; },
                     [] {}};
  Transition offToOn{State::off, State::on, Event::turnOn, [] { return true; },
                     [] {}};

  StateMachine m{State::off, std::make_tuple(offToOn, onToOff)};

  ASSERT_EQ(State::off, m.currentState);

  m.trigger(Event::turnOff); // already off, state won't change
  EXPECT_EQ(State::off, m.currentState);

  m.trigger(Event::turnOn);
  EXPECT_EQ(State::on, m.currentState);

  m.trigger(Event::turnOn); // already on, state won't change
  EXPECT_EQ(State::on, m.currentState);

  m.trigger(Event::turnOff);
  EXPECT_EQ(State::off, m.currentState);
}

TEST(StateMachineTests, transitionWithGaurdAndActions) {
  enum class State { on, off };
  enum class Event { turnOn, turnOff };

  using namespace susml;

  bool readyForOn = false;
  bool readyForOff = false;

  std::vector<std::string> reports;

  Transition offToOn{State::off, State::on, Event::turnOn,
                     [&] { return readyForOn; },
                     [&] { reports.push_back("turnOn"); }};
  Transition onToOff(
      State::on, State::off, Event::turnOff, [&] { return readyForOff; },
      [&] { reports.push_back("turnOff"); });

  auto transitions = std::make_tuple(offToOn, onToOff);

  using StateMachine =
      susml::tuplebased::StateMachine<State, Event, decltype(transitions)>;

  StateMachine m{State::off, transitions};

  ASSERT_FALSE(readyForOn);
  ASSERT_FALSE(readyForOff);
  ASSERT_EQ(State::off, m.currentState);

  m.trigger(Event::turnOff); // wrong event
  EXPECT_EQ(State::off, m.currentState);

  m.trigger(Event::turnOn); // right event, but readyForOn is false
  EXPECT_EQ(State::off, m.currentState);

  readyForOn = true;

  m.trigger(Event::turnOn); // right event and readyForOn is true
  EXPECT_EQ(State::on, m.currentState);
  EXPECT_EQ(1, reports.size());
  EXPECT_EQ("turnOn", reports.back());

  m.trigger(Event::turnOff); // right event but readyForOff is false
  EXPECT_EQ(State::on, m.currentState);
  EXPECT_EQ(1, reports.size());

  readyForOff = true;

  m.trigger(Event::turnOff); // right event and readyForOff is true
  EXPECT_EQ(State::off, m.currentState);
  EXPECT_EQ(2, reports.size());
  EXPECT_EQ("turnOff", reports.back());
}

namespace encoder {
using susml::Transition;
using susml::tuplebased::StateMachine;

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

  return StateMachine<State, Event, decltype(transitions)>(State::idle,
                                                           transitions);
}

struct Fixture : public ::testing::Test {
  bool a = false;
  bool b = false;
  int delta = 0;

  template <typename... Args>
  void makeUpdate(StateMachine<Args...> &m, Update update) {
    a = update.newA;
    b = update.newB;
    m.trigger(Event::update);
  }

  template <typename... Args, typename... Updates>
  void trigger(StateMachine<Args...> m, Updates... updates) {
    (..., makeUpdate(m, updates));
  }
};

} // namespace encoder

using EncoderTestFixture = encoder::Fixture;

TEST_F(EncoderTestFixture, fullClockWise) {
  using namespace encoder;

  auto m = makeStateMachine(delta, a, b);

  // full clockwise
  trigger(m, Update{false, true}, // cw1
          Update{true, true},     // cw2
          Update{true, false},    // cw3
          Update{false, false}    // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(1, delta);
}

TEST_F(EncoderTestFixture, fullCounterClockwise) {
  using namespace encoder;

  auto m = makeStateMachine(delta, a, b);

  // full counterclockwise
  trigger(m, Update{true, false}, // ccw1
          Update{true, true},     // ccw2
          Update{false, true},    // ccw3
          Update{false, false}    // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(-1, delta);
}

TEST_F(EncoderTestFixture, halfwayClockwise) {
  using namespace encoder;

  auto m = makeStateMachine(delta, a, b);

  // halfway clockwise
  trigger(m, Update{false, true}, // cw1
          Update{true, true},     // cw2
          Update{true, false},    // cw3
          Update{true, true},     // cw2
          Update{false, true},    // cw1
          Update{false, false}    // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(0, delta);
}
TEST_F(EncoderTestFixture, halfwayCounterClockwise) {
  using namespace encoder;

  auto m = makeStateMachine(delta, a, b);

  // halfway counterclockwise
  trigger(m, Update{true, false}, // ccw1
          Update{true, true},     // ccw2
          Update{false, true},    // ccw3
          Update{true, true},     // ccw2
          Update{true, false},    // ccw1
          Update{false, false}    // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(0, delta);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}