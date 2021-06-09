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

#include "susml.hpp"

TEST(CompositeTests, controllerAndSubsystem) {

  struct System {
    using Guard = std::function<bool()>;
    using Action = std::function<void()>;

    enum class ControllerState { on, off };
    enum class ControllerEvent { turnOn, turnOff };
    using Controller = susml::StateMachine<
        susml::Transition<ControllerState, ControllerEvent, Guard, Action>>;

    enum class SubsystemState { off, idle, running };
    enum class SubsystemEvent { turnOn, run, finish, turnOff };
    using Subsystem = susml::StateMachine<
        susml::Transition<SubsystemState, SubsystemEvent, Guard, Action>>;

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