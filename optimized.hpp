
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

#ifndef SUSML_OPTIMIZED_HPP
#define SUSML_OPTIMIZED_HPP

#include <algorithm>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace susml::optimized {

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

namespace evaluate {
/* Action execution
 */
constexpr void executeActions() {}

template <typename FirstAction, typename... OtherActions>
constexpr void executeActions(FirstAction &a, OtherActions &...as) {
  a();
  executeActions(as...);
}

template <typename... Actions>
constexpr void executeActionTuple(std::tuple<Actions...> &actions) {
  if constexpr (sizeof...(Actions) > 0) {
    std::apply(executeActions<Actions...>, actions);
  }
}
} // namespace evaluate

template <typename StateT, typename EventT, typename GuardsT = std::tuple<>,
          typename ActionsT = std::tuple<>>
class Transition {
public:
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
    return std::apply([&](const auto &...g) { return (... & g()); }, guards);
  }
  constexpr void executeActions() { evaluate::executeActionTuple(actions); }
};

// TODO test!

// TODO StateMachine with Transitions tuple

} // namespace susml::optimized

#endif
