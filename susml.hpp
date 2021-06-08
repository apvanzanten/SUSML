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
          typename GuardContainer = std::vector<Guard>,
          typename ActionContainer = std::vector<Action>>
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
  using GuardType = Guard;
  using ActionType = Action;
  using GuardContainerType = GuardContainer;
  using ActionContainerType = ActionContainer;

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
    std::for_each(actions.begin(), actions.end(), [](const Action &a) { a(); });
  }

  constexpr static Transition from(State source) {
    Transition t{};
    t.source = source;
    return t;
  }

  constexpr Transition &to(State target) {
    this->target = target;
    return *this;
  }
  constexpr Transition &triggeredBy(Event event) {
    this->event = event;
    return *this;
  }
  constexpr Transition &guardedBy(GuardContainer guards) {
    this->guards = std::move(guards);
    return *this;
  }
  constexpr Transition &calls(ActionContainer actions) {
    this->actions = std::move(actions);
    return *this;
  }
};

template <typename Transition,
          typename TransitionContainer = std::vector<Transition>>
class StateMachine {
  static_assert(
      std::is_same<Transition, typename TransitionContainer::value_type>::value,
      "TransitionContainer should be Container of Transition");

  using State = typename Transition::StateType;
  using Event = typename Transition::EventType;

public:
  using StateType = State;
  using EventType = Event;
  using TransitionType = Transition;
  using TransitionContainerType = TransitionContainer;

private:
  TransitionContainer mTransitions;
  State mState;

public:
  constexpr StateMachine() = default;
  constexpr StateMachine(const StateMachine &other) = default;
  constexpr StateMachine(StateMachine &&other) = default;
  constexpr StateMachine &operator=(const StateMachine &other) = default;
  constexpr StateMachine &operator=(StateMachine &&other) = default;

  constexpr StateMachine(TransitionContainer transitions, State initialState)
      : mTransitions(std::move(transitions)), mState(initialState) {}

  constexpr State state() const { return mState; }
  constexpr const auto &transitions() const { return mTransitions; }

  constexpr void trigger(Event event) {
    const auto it = std::find_if(mTransitions.begin(), mTransitions.end(),
                                 [event, this](const Transition &t) {
                                   return t.source == mState &&
                                          t.event == event && t.checkGuards();
                                 });

    if (it != mTransitions.end()) {
      const auto &t = *it;
      t.executeActions();
      mState = t.target;
    }
  }

  constexpr static StateMachine withInitialState(State initialState) {
    StateMachine m{};
    m.mState = initialState;
    return m;
  }
  constexpr StateMachine &withTransitions(TransitionContainer transitions) {
    mTransitions = std::move(transitions);
    return *this;
  }
};
} // namespace susml

#endif
