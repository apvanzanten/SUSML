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

#include "minimal/StateMachine.hpp"
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

using Guard = bool (*)();
using Action = void (*)();

using Transition = susml::minimal::Transition<State, Event, Guard, Action,
                                     std::vector<Guard>, std::vector<Action>>;
using StateMachine = susml::minimal::StateMachine<Transition, std::vector<Transition>>;

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

  EXPECT_EQ(m.currentState, INITIAL_STATE);
  EXPECT_EQ(m.currentState, State::off);

  m.trigger(Event::turnOn);
  EXPECT_EQ(m.currentState, State::on);
  EXPECT_EQ(Guards::numUnitGuardCalled, 1);
  EXPECT_EQ(Actions::numUnitActionCalled, 2);

  m.trigger(Event::turnOn);
  EXPECT_EQ(m.currentState, State::on);
  EXPECT_EQ(Guards::numUnitGuardCalled, 1);
  EXPECT_EQ(Actions::numUnitActionCalled, 3);

  m.trigger(Event::turnOff);
  EXPECT_EQ(m.currentState, State::off);
  EXPECT_EQ(Guards::numUnitGuardCalled, 2);
  EXPECT_EQ(Actions::numUnitActionCalled, 4);

  m.trigger(Event::turnOff);
  EXPECT_EQ(m.currentState, State::off);
  EXPECT_EQ(Guards::numUnitGuardCalled, 2);
  EXPECT_EQ(Actions::numUnitActionCalled, 4);
}

TEST(CompositeTests, controllerAndSubsystem) {
  struct System {
    using Guard = std::function<bool()>;
    using Action = std::function<void()>;

    enum class ControllerState { on, off };
    enum class ControllerEvent { turnOn, turnOff };
    using ControllerTransition =
        susml::minimal::Transition<ControllerState, ControllerEvent, Guard, Action,
                          std::vector<Guard>, std::vector<Action>>;
    using Controller = susml::minimal::StateMachine<ControllerTransition,
                                           std::vector<ControllerTransition>>;

    enum class SubsystemState { off, idle, running };
    enum class SubsystemEvent { turnOn, run, finish, turnOff };
    using SubsystemTransition =
        susml::minimal::Transition<SubsystemState, SubsystemEvent, Guard, Action,
                          std::vector<Guard>, std::vector<Action>>;
    using Subsystem = susml::minimal::StateMachine<SubsystemTransition,
                                          std::vector<SubsystemTransition>>;

    // forward declaration to allow references to eachother
    Subsystem subsys{{
                         {SubsystemState::off,
                          SubsystemEvent::turnOn,
                          {}, // no guard
                          {}, // no action
                          SubsystemState::idle},
                         {SubsystemState::idle,
                          SubsystemEvent::run,
                          {}, // no guard
                          {}, // no action
                          SubsystemState::running},
                         {SubsystemState::running,
                          SubsystemEvent::finish,
                          {}, // no guard
                          {}, // no action
                          SubsystemState::idle},
                         {SubsystemState::idle,
                          SubsystemEvent::turnOff,
                          {}, // no guard
                          {}, // no action
                          SubsystemState::off},
                     },
                     SubsystemState::off};

    Controller ctrl{{{ControllerState::off,
                      ControllerEvent::turnOn,
                      {}, // no guards
                      {[&] { subsys.trigger(SubsystemEvent::turnOn); }},
                      ControllerState::on},
                     {ControllerState::on,
                      ControllerEvent::turnOff,
                      {[&]() -> bool {
                        return subsys.currentState == SubsystemState::idle;
                      }},
                      {[&] { subsys.trigger(SubsystemEvent::turnOff); }},
                      ControllerState::off}},
                    ControllerState::off};
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}