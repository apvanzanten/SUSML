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

#ifndef SUSML_HPP
#define SUSML_HPP

#include <algorithm>
#include <type_traits>
#include <vector>

namespace susml {

// Transition is defined separately outside of Statemachine, such that the user
// can specify what TransitionContainer to use in StateMachine.
template <typename State,               // must be static_cast-able to int
          typename Event,               // must be static_cast-able to int
          typename Guard = bool (*)(),  // Should be invocable bool()
          typename Action = void (*)(), // should be invocable void()
          typename GuardContainer =
              std::vector<Guard>, // should be Container of Guard
          typename ActionContainer =
              std::vector<Action>> // should be Container of Action
struct Transition {
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

  using StateType = State;
  using EventType = Event;

  State source;
  Event event;
  GuardContainer guards;
  ActionContainer actions;
  State target;

  constexpr bool checkGuards() const {
    return std::all_of(guards.begin(), guards.end(),
                       [](const Guard &g) { return g(); });
  }

  constexpr void executeActions() const {
    for (const auto &a : actions) {
      a();
    }
  }
};

template <typename Transition,
          typename TransitionContainer = std::vector<Transition>>
class StateMachine {
  static_assert(
      std::is_same<Transition, typename TransitionContainer::value_type>::value,
      "TransitionContainer should be Container of Transition");

public:
  using TransitionContainerType = TransitionContainer;

private:
  using State = typename Transition::StateType;
  using Event = typename Transition::EventType;

  const TransitionContainer transitions;
  State current_state;

public:
  constexpr StateMachine(TransitionContainer transitions, State initialState)
      : transitions(transitions), current_state(initialState) {}

  constexpr State getState() const { return current_state; }

  constexpr auto getNumTransitions() const { return transitions.size(); }

  void trigger(Event event) {
    const auto it = std::find_if(transitions.begin(), transitions.end(),
                                 [&event, this](const Transition &t) {
                                   return t.source == current_state &&
                                          t.event == event && t.checkGuards();
                                 });

    if (it != transitions.end()) {
      const auto &t = *it;
      t.executeActions();
      current_state = t.target;
    }
  }
};
} // namespace susml

#endif
