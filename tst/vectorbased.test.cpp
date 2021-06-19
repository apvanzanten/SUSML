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

#include "Transition.hpp"
#include "factory.hpp"
#include "vectorbased.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <vector>

namespace Guards {
int numUnitGuardCalled = 0;
bool countGuard() {
  numUnitGuardCalled++;
  return true;
}
} // namespace Guards

namespace Actions {
int numUnitActionCalled = 0;
void countAction() { numUnitActionCalled++; }
} // namespace Actions

enum class State { off, on };
const State INITIAL_STATE = State::off;

enum class Event { turnOn, turnOff };

using Guard = std::function<bool()>;
using Action = std::function<void()>;

using Transition = susml::Transition<State, Event, Guard, Action>;
using StateMachine = susml::vectorbased::StateMachine<Transition>;

auto createBasicMachine() {
  return StateMachine{
      INITIAL_STATE,
      {{
           State::off,         // transition from state off
           State::on,          // transition to state on
           Event::turnOn,      // transition in response to turnOn event
           Guards::countGuard, // transition only if countGuard returns true
           [&] {
             Actions::countAction();
             Actions::countAction();
           } // on transition, call countAction twice
       },
       {
           State::on,           // transition from state on
           State::off,          // transition to state off
           Event::turnOff,      // transition in response to turnOff
                                // event
           Guards::countGuard,  // transition only if countGuard returns true
           Actions::countAction // on transition, call countAction
       },
       {
           State::on,            // transition from state on
           State::on,            // transition to state on
           Event::turnOn,        // transition in response to turnOn event
           [] { return true; },  // transition always
           Actions::countAction, // on transition, call countAction
       }}};
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
  using namespace susml::factory;

  struct System {
    using Guard = std::function<bool()>;
    using Action = std::function<void()>;

    const std::function<bool()> NoGuard = [] { return true; };
    const std::function<void()> NoAction = [] {};

    enum class ControllerState { on, off };
    enum class ControllerEvent { turnOn, turnOff };
    using ControllerTransition =
        susml::Transition<ControllerState, ControllerEvent, Guard, Action>;
    using Controller = susml::vectorbased::StateMachine<ControllerTransition>;

    enum class SubsystemState { off, idle, running };
    enum class SubsystemEvent { turnOn, run, finish, turnOff };
    using SubsystemTransition =
        susml::Transition<SubsystemState, SubsystemEvent, Guard, Action>;
    using Subsystem = susml::vectorbased::StateMachine<SubsystemTransition>;

    // forward declaration to allow references to eachother
    Subsystem subsys{
        SubsystemState::off,
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
             .make()}
    };

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
             .If(std::function(
                 [&] { return subsys.currentState == SubsystemState::idle; }))
             .Do(std::function(
                 [&] { subsys.trigger(SubsystemEvent::turnOff); }))
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}