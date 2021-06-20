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

#include "common.hpp"
#include "factory.hpp"
#include "vectorbased.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <vector>

using susml::vectorbased::StateMachine;

TEST(BasicTest, basic) {
  enum class State { off, on };
  enum class Event { turnOn, turnOff };

  using Guard        = std::function<bool()>;
  using Action       = std::function<void()>;
  using Transition   = susml::Transition<State, Event, Guard, Action>;
  using StateMachine = StateMachine<Transition>;

  std::size_t numActionCalled = 0;
  std::size_t numGuardCalled  = 0;
  auto        countedGuard    = [&numGuardCalled] {
    numGuardCalled++;
    return true;
  };
  auto countedAction = [&numActionCalled] { numActionCalled++; };

  auto m = StateMachine{State::off,
                        {{
                             State::off,    // transition from state off
                             State::on,     // transition to state on
                             Event::turnOn, // transition in response to turnOn event
                             countedGuard,  // transition only if countGuard returns true
                             [&] {
                               countedAction();
                               countedAction();
                             } // on transition, call countAction twice
                         },
                         {
                             State::on,      // transition from state on
                             State::off,     // transition to state off
                             Event::turnOff, // transition in response to turnOff
                                             // event
                             countedGuard,   // transition only if countGuard returns true
                             countedAction   // on transition, call countAction
                         },
                         {
                             State::on,           // transition from state on
                             State::on,           // transition to state on
                             Event::turnOn,       // transition in response to turnOn event
                             [] { return true; }, // transition always
                             countedAction,       // on transition, call countAction
                         }}};

  EXPECT_EQ(m.currentState, State::off);

  m.trigger(Event::turnOn);
  EXPECT_EQ(m.currentState, State::on);
  EXPECT_EQ(numGuardCalled, 1);
  EXPECT_EQ(numActionCalled, 2);

  m.trigger(Event::turnOn);
  EXPECT_EQ(m.currentState, State::on);
  EXPECT_EQ(numGuardCalled, 1);
  EXPECT_EQ(numActionCalled, 3);

  m.trigger(Event::turnOff);
  EXPECT_EQ(m.currentState, State::off);
  EXPECT_EQ(numGuardCalled, 2);
  EXPECT_EQ(numActionCalled, 4);

  m.trigger(Event::turnOff);
  EXPECT_EQ(m.currentState, State::off);
  EXPECT_EQ(numGuardCalled, 2);
  EXPECT_EQ(numActionCalled, 4);
}

TEST(CompositeTests, controllerAndSubsystem) {
  using namespace susml::factory;

  struct System {
    using Guard  = std::function<bool()>;
    using Action = std::function<void()>;

    const std::function<bool()> NoGuard  = [] { return true; };
    const std::function<void()> NoAction = [] {};

    enum class ControllerState { on, off };
    enum class ControllerEvent { turnOn, turnOff };
    using ControllerTransition = susml::Transition<ControllerState, ControllerEvent, Guard, Action>;
    using Controller           = StateMachine<ControllerTransition>;

    enum class SubsystemState { off, idle, running };
    enum class SubsystemEvent { turnOn, run, finish, turnOff };
    using SubsystemTransition = susml::Transition<SubsystemState, SubsystemEvent, Guard, Action>;
    using Subsystem           = StateMachine<SubsystemTransition>;

    // forward declaration to allow references to eachother
    Subsystem subsys{SubsystemState::off,
                     {From(SubsystemState::off)
                          .To(SubsystemState::idle)
                          .On(SubsystemEvent::turnOn)
                          .If(NoGuard)
                          .Do(NoAction)
                          .make(),
                      From(SubsystemState::idle)
                          .To(SubsystemState::running)
                          .On(SubsystemEvent::run)
                          .If(NoGuard)
                          .Do(NoAction)
                          .make(),
                      From(SubsystemState::running)
                          .To(SubsystemState::idle)
                          .On(SubsystemEvent::finish)
                          .If(NoGuard)
                          .Do(NoAction)
                          .make(),
                      From(SubsystemState::idle)
                          .To(SubsystemState::off)
                          .On(SubsystemEvent::turnOff)
                          .If(NoGuard)
                          .Do(NoAction)
                          .make()}};

    Controller ctrl{
        ControllerState::off,
        {From(ControllerState::off)
             .To(ControllerState::on)
             .On(ControllerEvent::turnOn)
             .If(NoGuard)
             .Do(std::function([&] { subsys.trigger(SubsystemEvent::turnOn); }))
             .make(),
         From(ControllerState::on)
             .To(ControllerState::off)
             .On(ControllerEvent::turnOff)
             .If(std::function([&] { return subsys.currentState == SubsystemState::idle; }))
             .Do(std::function([&] { subsys.trigger(SubsystemEvent::turnOff); }))
             .make()}};
  } system;

  using CtrlState = System::ControllerState;
  using CtrlEvent = System::ControllerEvent;
  using SubSState = System::SubsystemState;
  using SubSEvent = System::SubsystemEvent;

  system.ctrl.trigger(CtrlEvent::turnOn);
  EXPECT_EQ(CtrlState::on, system.ctrl.currentState);
  EXPECT_EQ(SubSState::idle, system.subsys.currentState);

  system.subsys.trigger(SubSEvent::run);
  EXPECT_EQ(SubSState::running, system.subsys.currentState);

  system.ctrl.trigger(CtrlEvent::turnOff);
  EXPECT_EQ(CtrlState::on, system.ctrl.currentState);
  EXPECT_EQ(SubSState::running, system.subsys.currentState);

  system.subsys.trigger(SubSEvent::finish);
  EXPECT_EQ(SubSState::idle, system.subsys.currentState);

  system.ctrl.trigger(CtrlEvent::turnOff);
  EXPECT_EQ(CtrlState::off, system.ctrl.currentState);
  EXPECT_EQ(SubSState::off, system.subsys.currentState);
}

namespace EncoderEventBased {
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

auto makeStateMachine(int &delta) {
  using namespace susml::factory;

  auto Fn       = [](auto e) { return std::function(e); };
  auto NoAction = Fn([] {});

  std::vector transitions = {
      From(State::idle).To(State::clockwise1).On(Event::updateB).Do(NoAction).make(),
      From(State::clockwise1).To(State::idle).On(Event::updateB).Do(NoAction).make(),
      From(State::clockwise1).To(State::clockwise2).On(Event::updateA).Do(NoAction).make(),
      From(State::clockwise2).To(State::clockwise1).On(Event::updateA).Do(NoAction).make(),
      From(State::clockwise2).To(State::clockwise3).On(Event::updateB).Do(NoAction).make(),
      From(State::clockwise3).To(State::clockwise2).On(Event::updateB).Do(NoAction).make(),
      From(State::clockwise3).To(State::idle).On(Event::updateA).Do(Fn([&] { delta++; })).make(),
      From(State::idle).To(State::counterclockwise1).On(Event::updateA).Do(NoAction).make(),
      From(State::counterclockwise1).To(State::idle).On(Event::updateA).Do(NoAction).make(),
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

  return StateMachine<decltype(transitions)::value_type>{State::idle, transitions};
}
} // namespace EncoderEventBased

TEST(EncoderEventBasedTests, fullClockWise) {
  using namespace EncoderEventBased;

  int  delta = 0;
  auto m     = makeStateMachine(delta);

  m.trigger(Event::updateB); // cw1
  m.trigger(Event::updateA); // cw2
  m.trigger(Event::updateB); // cw3
  m.trigger(Event::updateA); // idle

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(1, delta);
}

TEST(EncoderEventBasedTests, fullCounterClockWise) {
  using namespace EncoderEventBased;

  int  delta = 0;
  auto m     = makeStateMachine(delta);

  m.trigger(Event::updateA); // ccw1
  m.trigger(Event::updateB); // ccw2
  m.trigger(Event::updateA); // ccw3
  m.trigger(Event::updateB); // idle

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(-1, delta);
}

TEST(EncoderEventBasedTests, halfwayClockwise) {
  using namespace EncoderEventBased;

  int  delta = 0;
  auto m     = makeStateMachine(delta);

  m.trigger(Event::updateB); // cw1
  m.trigger(Event::updateA); // cw2
  m.trigger(Event::updateB); // cw3
  m.trigger(Event::updateB); // cw2
  m.trigger(Event::updateA); // cw1
  m.trigger(Event::updateB); // idle

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(0, delta);
}

TEST(EncoderEventBasedTests, halfwayCounterClockwise) {
  using namespace EncoderEventBased;

  int  delta = 0;
  auto m     = makeStateMachine(delta);

  m.trigger(Event::updateA); // ccw1
  m.trigger(Event::updateB); // ccw2
  m.trigger(Event::updateA); // ccw3
  m.trigger(Event::updateA); // ccw2
  m.trigger(Event::updateB); // ccw1
  m.trigger(Event::updateA); // idle

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(0, delta);
}

namespace EncoderGuardBased {
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

struct Update {
  bool newA = false;
  bool newB = false;
};

struct Fixture : public ::testing::Test {
  bool a     = false;
  bool b     = false;
  int  delta = 0;

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
} // namespace EncoderGuardBased

using EncoderGuardBasedFixture = EncoderGuardBased::Fixture;

TEST_F(EncoderGuardBasedFixture, fullClockWise) {
  using namespace EncoderGuardBased;

  auto m = makeStateMachine(delta, a, b);

  // full clockwise
  trigger(m,
          Update{false, true}, // cw1
          Update{true, true},  // cw2
          Update{true, false}, // cw3
          Update{false, false} // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(1, delta);
}

TEST_F(EncoderGuardBasedFixture, fullCounterClockwise) {
  using namespace EncoderGuardBased;

  auto m = makeStateMachine(delta, a, b);

  // full counterclockwise
  trigger(m,
          Update{true, false}, // ccw1
          Update{true, true},  // ccw2
          Update{false, true}, // ccw3
          Update{false, false} // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(-1, delta);
}

TEST_F(EncoderGuardBasedFixture, halfwayClockwise) {
  using namespace EncoderGuardBased;

  auto m = makeStateMachine(delta, a, b);

  // halfway clockwise
  trigger(m,
          Update{false, true}, // cw1
          Update{true, true},  // cw2
          Update{true, false}, // cw3
          Update{true, true},  // cw2
          Update{false, true}, // cw1
          Update{false, false} // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(0, delta);
}
TEST_F(EncoderGuardBasedFixture, halfwayCounterClockwise) {
  using namespace EncoderGuardBased;

  auto m = makeStateMachine(delta, a, b);

  // halfway counterclockwise
  trigger(m,
          Update{true, false}, // ccw1
          Update{true, true},  // ccw2
          Update{false, true}, // ccw3
          Update{true, true},  // ccw2
          Update{true, false}, // ccw1
          Update{false, false} // idle
  );

  EXPECT_EQ(State::idle, m.currentState);
  EXPECT_EQ(0, delta);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}