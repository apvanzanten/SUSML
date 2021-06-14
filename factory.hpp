#ifndef FACTORY_HPP
#define FACTORY_HPP
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

#include "susml.hpp"
#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

namespace susml::factory {
struct NoneType {
  constexpr bool operator==(const NoneType &) const { return true; }
  constexpr bool operator!=(const NoneType &) const { return false; }
};

template <typename StateT = NoneType, typename EventT = NoneType,
          typename GuardT = NoneType, typename ActionT = NoneType>
struct PartialTransition {
  using State = StateT;
  using Event = EventT;
  using Guard = GuardT;
  using Action = ActionT;
  using Guards = std::initializer_list<Guard>;
  using Actions = std::initializer_list<Action>;

  State source;
  Event event;
  Guards guards;
  Actions actions;
  State target;

  template <typename NewState> constexpr auto From(NewState newSource) const {
    if constexpr (std::is_same<State, NewState>::value) {
      // we already have a target (of the same type), keep it
      return PartialTransition{newSource, event, guards, actions, target};
    }
    // we need to set a new target, we will create a self-loop
    return PartialTransition<NewState, Event, Guard, Action>{
        newSource, event, guards, actions, newSource};
  }

  template <typename NewState> constexpr auto To(NewState newTarget) const {
    if constexpr (std::is_same<State, NewState>::value) {
      // we already have a source (of the same type), keep it
      return PartialTransition<State, Event, Guard, Action>{
          source, event, guards, actions, newTarget};
    }
    // we need to set a new source, we will create a self-loop
    return PartialTransition<NewState, Event, Guard, Action>{
        newTarget, event, guards, actions, newTarget};
  }

  template <typename NewEvent> constexpr auto On(NewEvent newEvent) const {
    return PartialTransition<State, NewEvent, Guard, Action>{
        source, newEvent, guards, actions, target};
  }

  template <typename NewGuard = bool (*)()>
  constexpr auto If(std::initializer_list<NewGuard> newGuards = {}) const {
    return PartialTransition<State, Event, NewGuard, Action>{
        source, event, newGuards, actions, target};
  }

  template <typename NewAction = void (*)()>
  constexpr auto Do(std::initializer_list<NewAction> newActions = {}) const {
    return PartialTransition<State, Event, Guard, NewAction>{
        source, event, guards, newActions, target};
  }

  constexpr auto
  // Transition<State, Event, Guard, Action,
  //                      std::vector<Guard>,
  //                      std::vector<Action>>
  make() const {
    constexpr bool hasGuards = !std::is_same<Guard, NoneType>::value;
    constexpr bool hasActions = !std::is_same<Action, NoneType>::value;

    if constexpr (!hasGuards) {
      return this->If().make();
    } else if constexpr (!hasActions) {
      return this->Do().make();
    } else if constexpr (hasGuards && hasActions) {
      using Guards = std::vector<Guard>;
      using Actions = std::vector<Action>;
      return Transition<State, Event, Guard, Action, Guards, Actions>{
          source, event, guards, actions, target};
    }
  }

  constexpr bool operator==(const PartialTransition &other) const {
    return ((source == other.source) && (target == other.target) &&
            (event == other.event) && (guards.size() == other.guards.size()) &&
            (actions.size() == other.actions.size()) &&
            std::equal(guards.begin(), guards.end(), other.guards.begin()) &&
            std::equal(actions.begin(), actions.end(), other.actions.begin()));
  }
  constexpr bool operator!=(const PartialTransition &other) const {
    return !(*this == other);
  }
};

template <typename State>
constexpr PartialTransition<State> From(State source) {
  // start as a self-loop, as we need to fill in something for the target state
  return {source, {}, {}, {}, source};
}

template <typename State> constexpr PartialTransition<State> To(State target) {
  // start as a self-loop, as we need to fill in something for the source state
  return {target, {}, {}, {}, target};
}

template <typename Event>
constexpr PartialTransition<NoneType, Event> On(Event newEvent) {
  return {{}, newEvent, {}, {}, {}};
}

template <typename Guard>
constexpr PartialTransition<NoneType, NoneType, Guard>
If(std::initializer_list<Guard> guards) {
  return {{}, {}, guards, {}, {}};
}

template <typename Action>
constexpr PartialTransition<NoneType, NoneType, NoneType, Action>
Do(std::initializer_list<Action> actions) {
  return {{}, {}, {}, actions, {}};
}

} // namespace susml::factory

#endif