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

#include "susml.hpp"

#include <iostream>

namespace Guards {
bool unitGuard() {
  std::cout << "called " << __func__ << std::endl;
  return true;
}
} // namespace Guards

namespace Actions {
void unitAction() { std::cout << "called " << __func__ << std::endl; }
} // namespace Actions

enum class States {
  off,
  on,
  NUM_STATES,
  INITIAL = off,
};

enum class Events { turnOn, turnOff };

using StateMachine = susml::StateMachine<States, Events>;
using Transition = StateMachine::Transition;
using TransitionList = StateMachine::TransitionList;
using TransitionMatrix = StateMachine::TransitionMatrix;

StateMachine m{TransitionMatrix{
    TransitionList{
        // transitions starting at state off
        {
            Events::turnOn,      // transition in response to turnOn event
            {Guards::unitGuard}, // transition only if unitGuard return true
            {Actions::unitAction,
             Actions::unitAction}, // on transition, call unitAction twice
            States::on             // transition to state on
        },
    },
    TransitionList{
        // transitions starting at state on
        {
            Events::turnOff,       // transition in response to turnOff
                                   // event
            {Guards::unitGuard},   // transition only if unitGuard returns true
            {Actions::unitAction}, // on transition, call unitAction
            States::off            // transition to state off
        },
        {
            Events::turnOn,        // transition in response to turnOn event
            {},                    // transition always
            {Actions::unitAction}, // on transition, call unitAction
            States::on             // transition to state on
        }}}};

int main() {
  bool result = true;
  auto check_eq = [&result](auto a, auto b) { result = result && (a == b); };

  std::cout << "StateMachineTest start!" << std::endl;

  std::cout << "starting at state: " << static_cast<int>(m.getState())
            << std::endl;
  std::cout << std::endl;
  check_eq(m.getState(), States::INITIAL);
  check_eq(m.getState(), States::off);

  std::cout << "triggering Events::turnOn at state #"
            << static_cast<int>(m.getState()) << std::endl;
  m.trigger(Events::turnOn);
  std::cout << "after event at state #" << static_cast<int>(m.getState())
            << std::endl;
  std::cout << std::endl;
  check_eq(m.getState(), States::on);

  std::cout << "triggering Events::turnOn at state #"
            << static_cast<int>(m.getState()) << std::endl;
  m.trigger(Events::turnOn);
  std::cout << "after event at state #" << static_cast<int>(m.getState())
            << std::endl;
  std::cout << std::endl;
  check_eq(m.getState(), States::on);

  std::cout << "triggering Events::turnOff at state #"
            << static_cast<int>(m.getState()) << std::endl;
  m.trigger(Events::turnOff);
  std::cout << "after event at state #" << static_cast<int>(m.getState())
            << std::endl;
  std::cout << std::endl;
  check_eq(m.getState(), States::off);

  std::cout << "triggering Events::turnOff at state #"
            << static_cast<int>(m.getState()) << std::endl;
  m.trigger(Events::turnOff);
  std::cout << "after event at state #" << static_cast<int>(m.getState())
            << std::endl;
  std::cout << std::endl;
  check_eq(m.getState(), States::off);

  std::cout << "test result: " << (result ? "PASS" : "FAIL") << std::endl;

  return result ? 0 : 1;
}