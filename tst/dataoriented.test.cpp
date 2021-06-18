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

#include "Transition.hpp"
#include "dataoriented/StateMachine.hpp"
#include "gtest/gtest.h"

TEST(StateMachineTests, basicOnOff) {
  using susml::dataoriented::StateMachine;

  enum class State { on, off };
  enum class Event { turnOn, turnOff };

  auto Fn = [](auto e) { return std::function(e); };

  bool isReadytToTurnOn = false;

  int numTimesTurnedOn = 0;
  int numTimesTurnedOff = 0;

  auto guardOffToOn = Fn([&] { return isReadytToTurnOn; });
  auto guardOnToOff = Fn([&] { return true; });

  auto actionOffToOn = Fn([&] { numTimesTurnedOn++; });
  auto actionOnToOff = Fn([&] { numTimesTurnedOff++; });

  auto m = StateMachine<State, Event>{State::off,
                                      {State::off, State::on},
                                      {State::on, State::off},
                                      {Event::turnOn, Event::turnOff},
                                      {guardOffToOn, guardOnToOff},
                                      {actionOffToOn, actionOnToOff}};

  m.trigger(Event::turnOn);
  EXPECT_EQ(State::off, m.currentState);
  EXPECT_EQ(0, numTimesTurnedOn);
  EXPECT_EQ(0, numTimesTurnedOff);

  isReadytToTurnOn = true;
  m.trigger(Event::turnOn);
  EXPECT_EQ(State::on, m.currentState);
  EXPECT_EQ(1, numTimesTurnedOn);
  EXPECT_EQ(0, numTimesTurnedOff);

  m.trigger(Event::turnOn);
  EXPECT_EQ(State::on, m.currentState);
  EXPECT_EQ(1, numTimesTurnedOn);
  EXPECT_EQ(0, numTimesTurnedOff);

  m.trigger(Event::turnOff);
  EXPECT_EQ(State::off, m.currentState);
  EXPECT_EQ(1, numTimesTurnedOn);
  EXPECT_EQ(1, numTimesTurnedOff);

  m.trigger(Event::turnOff);
  EXPECT_EQ(State::off, m.currentState);
  EXPECT_EQ(1, numTimesTurnedOn);
  EXPECT_EQ(1, numTimesTurnedOff);
}

TEST(StateMachineTests, fromTransitionsOnOff) {
  using namespace susml::dataoriented;
  using susml::Transition;

  enum class State { on, off };
  enum class Event { turnOn, turnOff };

  bool isReadytToTurnOn = false;

  int numTimesTurnedOn = 0;
  int numTimesTurnedOff = 0;

  auto m = fromTransitions(
      State::off,
      std::vector{Transition(State::off, State::on, Event::turnOn,
                             std::function([&] { return isReadytToTurnOn; }),
                             std::function([&] { numTimesTurnedOn++; })),
                  Transition(State::on, State::off, Event::turnOff,
                             std::function([&] { return true; }),
                             std::function([&] { numTimesTurnedOff++; }))});

m.trigger(Event::turnOn);
EXPECT_EQ(State::off, m.currentState);
EXPECT_EQ(0, numTimesTurnedOn);
EXPECT_EQ(0, numTimesTurnedOff);

isReadytToTurnOn = true;
m.trigger(Event::turnOn);
EXPECT_EQ(State::on, m.currentState);
EXPECT_EQ(1, numTimesTurnedOn);
EXPECT_EQ(0, numTimesTurnedOff);

m.trigger(Event::turnOn);
EXPECT_EQ(State::on, m.currentState);
EXPECT_EQ(1, numTimesTurnedOn);
EXPECT_EQ(0, numTimesTurnedOff);

m.trigger(Event::turnOff);
EXPECT_EQ(State::off, m.currentState);
EXPECT_EQ(1, numTimesTurnedOn);
EXPECT_EQ(1, numTimesTurnedOff);

m.trigger(Event::turnOff);
EXPECT_EQ(State::off, m.currentState);
EXPECT_EQ(1, numTimesTurnedOn);
EXPECT_EQ(1, numTimesTurnedOff);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}