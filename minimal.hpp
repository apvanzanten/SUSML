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

#ifndef MINIMAL_HPP
#define MINIMAL_HPP

#include <type_traits>

namespace susml::minimal {

// Transition is defined separately outside of Statemachine, such that the user
// can specify what TransitionContainer to use in StateMachine.
template <typename StateT, typename EventT,
          typename GuardT,  // must be invocable bool()
          typename ActionT, // must be invocable void()
          typename GuardContainerT, typename ActionContainerT>
struct Transition {
  using State = StateT;
  using Event = EventT;
  using Guard = GuardT;
  using Action = ActionT;
  using GuardContainer = GuardContainerT;
  using ActionContainer = ActionContainerT;

  static_assert(std::is_invocable<Guard>::value, "Guard should be invocable");
  static_assert(std::is_invocable<Action>::value, "Action should be invocable");
  static_assert(
      std::is_same<typename std::invoke_result<Guard>::type, bool>::value,
      "Guard should return bool.");
  static_assert(
      std::is_same<typename std::invoke_result<Action>::type, void>::value,
      "Action should return void.");
  static_assert(std::is_same<Guard, typename GuardContainer::value_type>::value,
                "GuardContainer should be Container of Guards.");
  static_assert(
      std::is_same<Action, typename ActionContainer::value_type>::value,
      "ActionContainer should be Container of Actions.");

  State source;
  Event event;
  GuardContainer guards;
  ActionContainer actions;
  State target;

  constexpr bool checkGuards() const {
    for (const auto &g : guards) {
      if (!g())
        return false;
    }
    return true;
  }

  constexpr void executeActions() {
    for (auto &a : actions) {
      a();
    }
  }
};

template <typename TransitionT, typename TransitionContainerT>
struct StateMachine {
  using Transition = TransitionT;
  using State = typename Transition::State;
  using Event = typename Transition::Event;
  using TransitionContainer = TransitionContainerT;

  static_assert(
      std::is_same<Transition, typename TransitionContainer::value_type>::value,
      "TransitionContainer should be Container of Transition");

  TransitionContainer transitions;
  State currentState;

  constexpr void trigger(const Event &event) {
    for (auto &t : transitions) {
      if (t.source == currentState && t.event == event && t.checkGuards()) {
        t.executeActions();
        currentState = t.target;
        return;
      }
    }
  }
};
} // namespace susml::minimal

#endif
