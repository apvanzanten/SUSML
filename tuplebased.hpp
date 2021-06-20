
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

#ifndef TUPLEBASED_HPP
#define TUPLEBASED_HPP

#include <algorithm>
#include <type_traits>

#include "common.hpp"

namespace susml::tuplebased {

namespace validate {
/* Transitions type validation
 */

template <typename>
struct IsTransitionTypeImpl : std::false_type {};

template <typename StateT, typename EventT, typename GuardT, typename Actions>
struct IsTransitionTypeImpl<Transition<StateT, EventT, GuardT, Actions>> : std::true_type {};

template <typename T>
constexpr bool isTransitionType() {
  return IsTransitionTypeImpl<T>::value;
}

template <typename TransitionTuple, std::size_t... Indices>
constexpr bool areTypesAtIndicesTransitionTypes(std::index_sequence<Indices...>) {
  return (isTransitionType<typename std::tuple_element<Indices, TransitionTuple>::type>() && ...);
}

template <typename Transition, typename State, typename Event>
constexpr bool hasTransitionTypeStateAndEvent() {
  return std::is_same<typename Transition::State, State>::value &&
         std::is_same<typename Transition::Event, Event>::value;
}

template <typename TransitionTuple, typename State, typename Event, std::size_t... Indices>
constexpr bool isValidTransitionTupleTypeImpl(const std::index_sequence<Indices...> &) {
  return (
      ... &&
      hasTransitionTypeStateAndEvent<typename std::tuple_element<Indices, TransitionTuple>::type,
                                     State,
                                     Event>());
}

template <typename TransitionTuple, typename State, typename Event>
constexpr bool isValidTransitionTupleType() {
  if constexpr (std::tuple_size<TransitionTuple>::value > 0) {
    constexpr auto indices = std::make_index_sequence<std::tuple_size<TransitionTuple>::value>();
    return isValidTransitionTupleTypeImpl<TransitionTuple, State, Event>(indices);
  }
  return false; // the tuple is empty, it needs at least one transition
}

} // namespace validate

template <typename StateT, typename EventT, typename TransitionsT>
struct StateMachine {
  using TransitionTuple = TransitionsT;
  using State           = StateT;
  using Event           = EventT;

  static_assert(validate::isValidTransitionTupleType<TransitionTuple, State, Event>(),
                "StateMachine needs at least one transition, and all "
                "transitions must have the correct State type and Event type.");

  State           currentState;
  TransitionTuple transitions;

  constexpr StateMachine(const State &initialState, const TransitionTuple &transitions)
      : currentState(initialState), transitions(transitions) {}

  constexpr void trigger(const Event &event) {
    constexpr std::size_t numTransitions = std::tuple_size<TransitionTuple>::value;
    triggerImpl(event, std::make_index_sequence<numTransitions>());
  }

  // helper functions
  template <typename Transition>
  constexpr bool isTakeableTransition(const Transition &transition, const Event &event) {
    if constexpr (Transition::HasGuard()) {
      return currentState == transition.source && event == transition.event && transition.guard();
    }
    if constexpr (!Transition::HasGuard()) {
      return currentState == transition.source && event == transition.event;
    }
  }

  template <typename Transition>
  constexpr bool takeTransitionIfAble(Transition &transition, const Event &event) {

    const bool isTakeable = isTakeableTransition(transition, event);
    if (isTakeable) {
      if constexpr (Transition::HasAction()) { transition.action(); }
      currentState = transition.target;
      return true;
    }
    return false;
  }

  template <std::size_t... Indices>
  constexpr bool triggerImpl(const Event &event, const std::index_sequence<Indices...> &) {
    return (... || takeTransitionIfAble(std::get<Indices>(transitions), event));
  }
};

} // namespace susml::tuplebased

#endif
