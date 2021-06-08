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
    Subsystem subsys =
        Subsystem::withInitialState(SubsystemState::off)
            .withTransitions({
                Subsystem::TransitionType::from(SubsystemState::off)
                    .to(SubsystemState::idle)
                    .triggeredBy(SubsystemEvent::turnOn),
                Subsystem::TransitionType::from(SubsystemState::idle)
                    .to(SubsystemState::running)
                    .triggeredBy(SubsystemEvent::run),
                Subsystem::TransitionType::from(SubsystemState::running)
                    .to(SubsystemState::idle)
                    .triggeredBy(SubsystemEvent::finish),
                Subsystem::TransitionType::from(SubsystemState::idle)
                    .to(SubsystemState::off)
                    .triggeredBy(SubsystemEvent::turnOff),
            });

    Controller ctrl =
        Controller::withInitialState(ControllerState::off)
            .withTransitions(
                {Controller::TransitionType::from(ControllerState::off)
                     .to(ControllerState::on)
                     .triggeredBy(ControllerEvent::turnOn)
                     .calls({[&] { subsys.trigger(SubsystemEvent::turnOn); }}),
                 Controller::TransitionType::from(ControllerState::on)
                     .to(ControllerState::off)
                     .triggeredBy(ControllerEvent::turnOff)
                     .guardedBy({[&]() -> bool {
                       return subsys.state() == SubsystemState::idle;
                     }})
                     .calls(
                         {[&] { subsys.trigger(SubsystemEvent::turnOff); }})});
  } system;

  using CtrlState = System::ControllerState;
  using CtrlEvent = System::ControllerEvent;
  using SubSState = System::SubsystemState;
  using SubSEvent = System::SubsystemEvent;

  system.ctrl.trigger(CtrlEvent::turnOn);
  EXPECT_EQ(CtrlState::on, system.ctrl.state());
  EXPECT_EQ(SubSState::idle, system.subsys.state());

  system.subsys.trigger(SubSEvent::run);
  EXPECT_EQ(SubSState::running, system.subsys.state());

  system.ctrl.trigger(CtrlEvent::turnOff);
  EXPECT_EQ(CtrlState::on, system.ctrl.state());
  EXPECT_EQ(SubSState::running, system.subsys.state());

  system.subsys.trigger(SubSEvent::finish);
  EXPECT_EQ(SubSState::idle, system.subsys.state());

  system.ctrl.trigger(CtrlEvent::turnOff);
  EXPECT_EQ(CtrlState::off, system.ctrl.state());
  EXPECT_EQ(SubSState::off, system.subsys.state());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}