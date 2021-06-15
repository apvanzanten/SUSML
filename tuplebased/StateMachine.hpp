
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

#ifndef TUPLEBASED_STATEMACHINE_HPP
#define TUPLEBASED_STATEMACHINE_HPP

#include <algorithm>
#include <bits/c++config.h>
#include <initializer_list>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace susml::tuplebased {

namespace validate {

/* Guard type validation
 */
template <typename Candidate> constexpr bool isGuardType() {
  // these two checks are put together with this constexpr-if because
  // std::invoke_result will break compilation otherwise in cases where
  // Candidate is not invocable
  if constexpr (std::is_invocable<Candidate>::value) {
    return std::is_same<typename std::invoke_result<Candidate>::type,
                        bool>::value;
  }
  return false;
}

template <typename GuardTuple, std::size_t... Indices>
constexpr bool areTypesAtIndicesGuardTypes(std::index_sequence<Indices...>) {
  return (
      isGuardType<typename std::tuple_element<Indices, GuardTuple>::type>() &&
      ...);
}

template <typename GuardTuple> constexpr bool isGuardTupleType() {
  auto indices = std::make_index_sequence<std::tuple_size<GuardTuple>::value>();
  return areTypesAtIndicesGuardTypes<GuardTuple>(indices);
}

/* Action type validation
 */
template <typename Candidate> constexpr bool isActionType() {
  // these two checks are put together with this constexpr-if because
  // std::invoke_result will break compilation otherwise in cases where
  // Candidate is not invocable
  if constexpr (std::is_invocable<Candidate>::value) {
    return std::is_same<typename std::invoke_result<Candidate>::type,
                        void>::value;
  }
  return false;
}

template <typename ActionTuple, std::size_t... Indices>
constexpr bool areTypesAtIndicesActionTypes(std::index_sequence<Indices...>) {
  return (
      isActionType<typename std::tuple_element<Indices, ActionTuple>::type>() &&
      ...);
}

template <typename ActionTuple> constexpr bool isActionTupleType() {
  auto indices =
      std::make_index_sequence<std::tuple_size<ActionTuple>::value>();
  return areTypesAtIndicesActionTypes<ActionTuple>(indices);
}

} // namespace validate

template <typename StateT, typename EventT, typename GuardsT = std::tuple<>,
          typename ActionsT = std::tuple<>>
struct Transition {
  using State = StateT;
  using Event = EventT;
  using Guards = GuardsT;
  using Actions = ActionsT;

  static_assert(validate::isGuardTupleType<Guards>(),
                "Guards should be tuple of invocables returning bool");
  static_assert(validate::isActionTupleType<Actions>(),
                "Actions should be tuple of invocables return void");

  State source;
  State target;
  Event event;
  Guards guards;
  Actions actions;

  constexpr Transition() = default;

  constexpr Transition(const State &s, const State &t, const Event &e,
                       const Guards &g = std::tuple<>(),
                       const Actions &a = std::tuple<>())
      : source(s), target(t), event(e), guards(g), actions(a) {}

  constexpr Transition(const Transition &other) = default;
  constexpr Transition(Transition &&other) = default;
  constexpr Transition &operator=(const Transition &other) = default;
  constexpr Transition &operator=(Transition &&other) = default;

  constexpr bool checkGuards() const {
    return std::apply([&](const auto &...g) { return (... && g()); }, guards);
  }
  constexpr void executeActions() {
    std::apply([&](auto &...a) { (..., a()); }, actions);
  }
};

namespace validate {
/* Transitions type validation
 */

template <typename> struct IsTransitionTypeImpl : std::false_type {};

template <typename StateT, typename EventT, typename GuardsT, typename ActionsT>
struct IsTransitionTypeImpl<Transition<StateT, EventT, GuardsT, ActionsT>>
    : std::true_type {};

template <typename T> constexpr bool isTransitionType() {
  return IsTransitionTypeImpl<T>::value;
}

template <typename TransitionTuple, std::size_t... Indices>
constexpr bool
areTypesAtIndicesTransitionTypes(std::index_sequence<Indices...>) {
  return (isTransitionType<
              typename std::tuple_element<Indices, TransitionTuple>::type>() &&
          ...);
}

template <typename Transition, typename State, typename Event>
constexpr bool hasTransitionTypeStateAndEvent() {
  return std::is_same<typename Transition::State, State>::value &&
         std::is_same<typename Transition::Event, Event>::value;
}

template <typename TransitionTuple, typename State, typename Event,
          std::size_t... Indices>
constexpr bool
isValidTransitionTupleTypeImpl(const std::index_sequence<Indices...> &) {
  return (... &&
          hasTransitionTypeStateAndEvent<
              typename std::tuple_element<Indices, TransitionTuple>::type,
              State, Event>());
}

template <typename TransitionTuple, typename State, typename Event>
constexpr bool isValidTransitionTupleType() {
  if constexpr (std::tuple_size<TransitionTuple>::value > 0) {
    constexpr auto indices =
        std::make_index_sequence<std::tuple_size<TransitionTuple>::value>();
    return isValidTransitionTupleTypeImpl<TransitionTuple, State, Event>(
        indices);
  }
  return false; // the tuple is empty, it needs at least one transition
}

} // namespace validate

template <typename StateT, typename EventT, typename TransitionsT>
class StateMachine {
public:
  using TransitionTuple = TransitionsT;
  using State = StateT;
  using Event = EventT;

  static_assert(
      validate::isValidTransitionTupleType<TransitionTuple, State, Event>(),
      "StateMachine needs at least one transition, and all "
      "transitions must have the correct State type and Event type.");

  constexpr StateMachine(const TransitionTuple &transitions,
                         const State &initialState)
      : mTransitions(transitions), mCurrentState(initialState) {}

  constexpr void trigger(const Event &event) {
    constexpr std::size_t numTransitions =
        std::tuple_size<TransitionTuple>::value;
    // std::cout << "trigger(" << static_cast<std::size_t>(event) << ")"
    //           << std::endl;
    triggerImpl(event, std::make_index_sequence<numTransitions>());
  }

  constexpr const TransitionTuple &transitions() const { return mTransitions; }
  constexpr const State &currentState() const { return mCurrentState; }
  constexpr void setState(State newState) { mCurrentState = newState; }

private:
  TransitionTuple mTransitions;
  State mCurrentState;

  template <typename Transition>
  constexpr bool isTakeableTransition(const Transition &transition,
                                      const Event &event) {
    // std::cout << "isTakeableTransition( "
    //           << static_cast<std::size_t>(mCurrentState)
    //           << " ?= " << static_cast<std::size_t>(transition.source) << " && "
    //           << std::boolalpha
    //           << transition.checkGuards() << "?"
    //           << " : " << static_cast<std::size_t>(transition.target) 
    //           << " )" << std::endl;
    return mCurrentState == transition.source && event == transition.event &&
           transition.checkGuards();
  }

  template <typename Transition>
  constexpr bool takeTransitionIfAble(Transition &transition,
                                      const Event &event) {

    // std::cout << "takeTransitionIfAble(" << std::endl;
    const bool isTakeable = isTakeableTransition(transition, event);
    if (isTakeable) {
      transition.executeActions();
      mCurrentState = transition.target;
      // std::cout << "->" << static_cast<std::size_t>(transition.target) << ")"
      //           << std::endl;
      return true;
    }
    // std::cout << ")" << std::endl;
    return false;
  }

  template <std::size_t... Indices>
  constexpr bool triggerImpl(const Event &event,
                             const std::index_sequence<Indices...> &) {
    // std::cout << "triggerImpl(" << static_cast<std::size_t>(event) << ", "
    //           << (... << Indices) << ")" << std::endl;
    return (... ||
            takeTransitionIfAble(std::get<Indices>(mTransitions), event));
  }
};

} // namespace susml::tuplebased

#endif
