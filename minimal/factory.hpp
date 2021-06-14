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

#include "StateMachine.hpp"
#include <initializer_list>
#include <type_traits>
#include <vector>

namespace susml::minimal::factory {
struct NoneType {
  constexpr bool operator==(const NoneType &) const { return true; }
  constexpr bool operator!=(const NoneType &) const { return false; }
};

template <typename StateT = NoneType, typename EventT = NoneType,
          typename GuardT = NoneType, typename ActionT = NoneType,
          typename GuardContainerT = NoneType,
          typename ActionContainerT = NoneType>
struct PartialTransition {
  using State = StateT;
  using Event = EventT;
  using Guard = GuardT;
  using Action = ActionT;
  using GuardContainer = GuardContainerT;
  using ActionContainer = ActionContainerT;

  static constexpr bool HasGuards = !std::is_same<Guard, NoneType>::value;
  static constexpr bool HasActions = !std::is_same<Action, NoneType>::value;

  State source;
  Event event;
  GuardContainer guards;
  ActionContainer actions;
  State target;

  template <typename NewState> constexpr auto From(NewState newSource) const {
    if constexpr (std::is_same<State, NewState>::value) {
      // we already have a target (of the same type), keep it
      return PartialTransition{newSource, event, guards, actions, target};
    }
    // we need to set a new target, we will create a self-loop
    return PartialTransition<NewState, Event, Guard, Action, GuardContainer,
                             ActionContainer>{newSource, event, guards, actions,
                                              newSource};
  }

  template <typename NewState> constexpr auto To(NewState newTarget) const {
    if constexpr (std::is_same<State, NewState>::value) {
      // we already have a source (of the same type), keep it
      return PartialTransition<State, Event, Guard, Action, GuardContainer,
                               ActionContainer>{source, event, guards, actions,
                                                newTarget};
    }
    // we need to set a new source, we will create a self-loop
    return PartialTransition<NewState, Event, Guard, Action, GuardContainer,
                             ActionContainer>{newTarget, event, guards, actions,
                                              newTarget};
  }

  template <typename NewEvent> constexpr auto On(NewEvent newEvent) const {
    return PartialTransition<State, NewEvent, Guard, Action, GuardContainer,
                             ActionContainer>{source, newEvent, guards, actions,
                                              target};
  }

  template <typename NewGuard,
            typename NewGuardContainer = std::vector<NewGuard>>
  constexpr PartialTransition<State, Event, NewGuard, Action, NewGuardContainer,
                              ActionContainer>
  If(std::initializer_list<NewGuard> newGuards) const {
    return {source, event, newGuards, actions, target};
  }

  template <typename NewGuard,
            typename NewGuardContainer = std::vector<NewGuard>>
  constexpr PartialTransition<State, Event, NewGuard, Action, NewGuardContainer,
                              ActionContainer>
  If(const std::vector<NewGuard> &newGuards) const {
    return {source, event, newGuards, actions, target};
  }

  constexpr auto If() const {
    using NewGuard = bool (*)();
    return If<NewGuard>({});
  }

  template <typename NewAction,
            typename NewActionContainer = std::vector<NewAction>>
  constexpr PartialTransition<State, Event, Guard, NewAction, GuardContainer,
                              NewActionContainer>
  Do(std::initializer_list<NewAction> newActions) const {
    return {source, event, guards, newActions, target};
  }

  template <typename NewAction,
            typename NewActionContainer = std::vector<NewAction>>
  constexpr PartialTransition<State, Event, Guard, NewAction, GuardContainer,
                              NewActionContainer>
  Do(const std::vector<NewAction> &newActions) const {
    return {source, event, guards, newActions, target};
  }

  constexpr auto Do() const {
    using NewAction = void (*)();
    return Do<NewAction>({});
  }

  constexpr auto make() const {
    if constexpr (!HasGuards) {
      return this->If().make();
    } else if constexpr (!HasActions) {
      return this->Do().make();
    } else if constexpr (HasGuards && HasActions) {
      return Transition<State, Event, Guard, Action, GuardContainer,
                        ActionContainer>{source, event, guards, actions,
                                         target};
    }
  }

  constexpr bool hasGuards() const { return HasGuards; }
  constexpr bool hasActions() const { return HasActions; }

  template <typename... Types>
  constexpr bool operator==(const PartialTransition<Types...> &) const {
    return false;
  }

  constexpr bool operator==(
      const PartialTransition<State, Event, Guard, Action, GuardContainer,
                              ActionContainer> &other) const {
    return (source == other.source) && (target == other.target) &&
           (event == other.event) && (guards == other.guards) &&
           (actions == other.actions);
  }

  template <typename... Types>
  constexpr bool operator!=(const PartialTransition<Types...> &other) const {
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

template <typename Guard, typename GuardContainer = std::vector<Guard>>
constexpr PartialTransition<NoneType, NoneType, Guard, NoneType, GuardContainer>
If(std::initializer_list<Guard> guards) {
  return {{}, {}, guards, {}, {}};
}

template <typename Action, typename ActionContainer = std::vector<Action>>
constexpr PartialTransition<NoneType, NoneType, NoneType, Action, NoneType,
                            ActionContainer>
Do(std::initializer_list<Action> actions) {
  return {{}, {}, {}, actions, {}};
}

} // namespace susml::minimal::factory

#endif