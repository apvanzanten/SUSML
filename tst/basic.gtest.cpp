// This file is part of Still Untitled State Machine Library (SUSML).
//    Copyright (C) 2020 A.P. van Zanten
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

#include "susml.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <vector>

namespace Guards {
int numUnitGuardCalled = 0;
bool unitGuard() {
  numUnitGuardCalled++;
  return true;
}
} // namespace Guards

namespace Actions {
int numUnitActionCalled = 0;
void unitAction() { numUnitActionCalled++; }
} // namespace Actions

enum class State { off, on };
const State INITIAL_STATE = State::off;

enum class Event { turnOn, turnOff };

using Transition = susml::Transition<State, Event>;
using StateMachine = susml::StateMachine<Transition>;

auto createBasicMachine() {
  return StateMachine{
      {{
           State::off,          // transition from state off
           Event::turnOn,       // transition in response to turnOn event
           {Guards::unitGuard}, // transition only if unitGuard return true
           {Actions::unitAction,
            Actions::unitAction}, // on transition, call unitAction twice
           State::on              // transition to state on
       },
       {
           State::on,             // transition from state on
           Event::turnOff,        // transition in response to turnOff
                                  // event
           {Guards::unitGuard},   // transition only if unitGuard returns true
           {Actions::unitAction}, // on transition, call unitAction
           State::off             // transition to state off
       },
       {
           State::on,             // transition from state on
           Event::turnOn,         // transition in response to turnOn event
           {},                    // transition always
           {Actions::unitAction}, // on transition, call unitAction
           State::on              // transition to state on
       }},
      INITIAL_STATE};
}

TEST(BasicTest, GoodWeather) {
  auto m = createBasicMachine();
  Guards::numUnitGuardCalled = 0;
  Actions::numUnitActionCalled = 0;

  EXPECT_EQ(m.getState(), INITIAL_STATE);
  EXPECT_EQ(m.getState(), State::off);

  m.trigger(Event::turnOn);
  EXPECT_EQ(m.getState(), State::on);
  EXPECT_EQ(Guards::numUnitGuardCalled, 1);
  EXPECT_EQ(Actions::numUnitActionCalled, 2);

  m.trigger(Event::turnOn);
  EXPECT_EQ(m.getState(), State::on);
  EXPECT_EQ(Guards::numUnitGuardCalled, 1);
  EXPECT_EQ(Actions::numUnitActionCalled, 3);

  m.trigger(Event::turnOff);
  EXPECT_EQ(m.getState(), State::off);
  EXPECT_EQ(Guards::numUnitGuardCalled, 2);
  EXPECT_EQ(Actions::numUnitActionCalled, 4);

  m.trigger(Event::turnOff);
  EXPECT_EQ(m.getState(), State::off);
  EXPECT_EQ(Guards::numUnitGuardCalled, 2);
  EXPECT_EQ(Actions::numUnitActionCalled, 4);
}

TEST(BasicTest, BadWeather) {
  // This test does nothing, as there is currently no known reasonable way to
  // make this code fail. That's pretty cool!
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}