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

#include "Transition.hpp"
#include <functional>
#include <type_traits>

namespace susml::factory {
template <typename StateT  = NoneType,
          typename EventT  = NoneType,
          typename GuardT  = NoneType,
          typename ActionT = NoneType>
struct PartialTransition {
  using State  = StateT;
  using Event  = EventT;
  using Guard  = GuardT;
  using Action = ActionT;

  static constexpr bool HasState() { return !isNoneType<State>(); }
  static constexpr bool HasEvent() { return !isNoneType<Event>(); }
  static constexpr bool HasGuard() { return !isNoneType<Guard>(); }
  static constexpr bool HasAction() { return !isNoneType<Action>(); }

  State  source;
  State  target;
  Event  event;
  Guard  guard;
  Action action;

  template <typename NewState>
  constexpr auto From(NewState newSource) const {
    if constexpr (std::is_same<State, NewState>::value) {
      // we already have a target (of the same type), keep it
      return PartialTransition{newSource, target, event, guard, action};
    }
    // we need to set a new target, we will create a self-loop
    return PartialTransition<NewState, Event, Guard, Action>{
        newSource, newSource, event, guard, action};
  }

  template <typename NewState>
  constexpr auto To(NewState newTarget) const {
    if constexpr (std::is_same<State, NewState>::value) {
      // we already have a source (of the same type), keep it
      return PartialTransition<State, Event, Guard, Action>{
          source, newTarget, event, guard, action};
    }
    // we need to set a new source, we will create a self-loop
    return PartialTransition<NewState, Event, Guard, Action>{
        newTarget, newTarget, event, guard, action};
  }

  template <typename NewEvent>
  constexpr auto On(NewEvent newEvent) const {
    return PartialTransition<State, NewEvent, Guard, Action>{
        source, target, newEvent, guard, action};
  }

  template <typename NewGuard = std::function<bool()>>
  constexpr PartialTransition<State, Event, NewGuard, Action> If(NewGuard newGuard) const {
    return {source, target, event, newGuard, action};
  }

  template <typename NewAction = std::function<void()>>
  constexpr PartialTransition<State, Event, Guard, NewAction> Do(NewAction newAction) const {
    return {source, target, event, guard, newAction};
  }

  constexpr auto If() const { return If<NoneType>({}); }
  constexpr auto Do() const { return Do<NoneType>({}); }

  constexpr auto make() const {
    static_assert(HasState() && HasEvent(),
                  "Transition must have at least State and Event types defined");
    return Transition<State, Event, Guard, Action>{source, target, event, guard, action};
  }

  template <typename... Types>
  constexpr bool operator==(const PartialTransition<Types...> &) const {
    return false;
  }

  constexpr bool operator==(const PartialTransition &other) const {
    return (source == other.source) && (target == other.target) && (event == other.event) &&
           (guard == other.guard) && (action == other.action);
  }

  template <typename... Types>
  constexpr bool operator!=(const PartialTransition<Types...> &other) const {
    return !(*this == other);
  }
};

template <typename State>
constexpr PartialTransition<State> From(State source) {
  // start as a self-loop, as we need to fill in something for the target state
  return {source, source, {}, {}, {}};
}

template <typename State>
constexpr PartialTransition<State> To(State target) {
  // start as a self-loop, as we need to fill in something for the source state
  return {target, target, {}, {}, {}};
}

template <typename Event>
constexpr PartialTransition<NoneType, Event> On(Event newEvent) {
  return {{}, {}, newEvent, {}, {}};
}

template <typename Guard>
constexpr PartialTransition<NoneType, NoneType, Guard> If(Guard guard) {
  return {{}, {}, {}, guard, {}};
}

template <typename Action>
constexpr PartialTransition<NoneType, NoneType, NoneType, Action> Do(Action action) {
  return {{}, {}, {}, {}, action};
}

} // namespace susml::factory

#endif