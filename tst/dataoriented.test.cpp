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

  bool isReadytToTurnOn = false;

  int numTimesTurnedOn = 0;
  int numTimesTurnedOff = 0;

  auto guardOffToOn = [&] { return isReadytToTurnOn; };
  auto guardOnToOff = [&] { return true; };

  auto actionOffToOn = [&] { numTimesTurnedOn++; };
  auto actionOnToOff = [&] { numTimesTurnedOff++; };

  using GuardVariant =
      std::variant<decltype(guardOffToOn), decltype(guardOnToOff)>;
  using ActionVariant =
      std::variant<decltype(actionOffToOn), decltype(actionOnToOff)>;

  auto m = StateMachine<State, Event, GuardVariant, ActionVariant, 2>{
      {State::off, State::on},         {State::on, State::off},
      {Event::turnOn, Event::turnOff}, {guardOffToOn, guardOnToOff},
      {actionOffToOn, actionOnToOff},  State::off};

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

  Transition offToOn(State::off, State::on, Event::turnOn, [&] { return isReadytToTurnOn; }, [&] { numTimesTurnedOn++; });
  Transition onToOff(State::on, State::off, Event::turnOff, [&] { return true; }, [&] { numTimesTurnedOff++; });

  auto m = fromTransitions(State::off, offToOn, onToOff);

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