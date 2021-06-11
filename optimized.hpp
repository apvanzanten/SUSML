
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

namespace validity {

template <typename Candidate> constexpr bool isGuardTypeValid() {
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
constexpr bool areGuardTypesAtIndicesValid(std::index_sequence<Indices...>) {
  (std::cout << ... << Indices) << std::endl;
  return (
      isGuardTypeValid<typename std::tuple_element<Indices, GuardTuple>::type>() &&
      ...);
}

template <typename GuardTuple> constexpr bool isValidGuardTupleType() {
  auto indices = std::make_index_sequence<std::tuple_size<GuardTuple>::value>();
  return areGuardTypesAtIndicesValid<GuardTuple>(indices);
}

constexpr bool isValidActions() { return true; }

template <typename ActionCandidate>
constexpr bool isValidAction(const ActionCandidate &) {
  // these two checks are put together with this constexpr-if because
  // std::invoke_result will break compilation otherwise in cases where
  // GuardCandidate is not invocable
  if constexpr (std::is_invocable<ActionCandidate>::value) {
    return std::is_same<typename std::invoke_result<ActionCandidate>::type,
                        void>::value;
  }
  return false;
}

template <typename ActionCandidate, typename... Args>
constexpr bool isValidActions(const ActionCandidate &gc, const Args &...args) {
  return isValidAction(gc) && isValidActions(args...);
}

template <typename... Args>
constexpr bool isValidActionTuple(const std::tuple<Args...> &t) {
  if constexpr ((sizeof...(Args) > 0)) {
    return std::apply(isValidActions<Args...>, t);
  }
  return true;
}
} // namespace validity

namespace evaluate {
constexpr bool checkGuards() { return true; }

template <typename FirstGuard, typename... OtherGuards>
constexpr bool checkGuards(const FirstGuard &g, const OtherGuards &...gs) {
  return g() && checkGuards(gs...);
}

template <typename... Guards>
constexpr bool checkGuards(const std::tuple<Guards...> &guards) {
  return (sizeof...(Guards) > 0) || std::apply(checkGuards<Guards...>, guards);
}

constexpr void executeActions() {}

template <typename FirstAction, typename... OtherActions>
constexpr void executeActions(FirstAction &a, OtherActions &...as) {
  a();
  executeActions(as...);
}

template <typename... Actions>
constexpr void executeActions(std::tuple<Actions...> &actions) {
  if constexpr (sizeof...(Actions) > 0) {
    std::apply(executeActions<Actions...>, actions);
  }
}

} // namespace evaluate

template <typename StateT, typename EventT, typename GuardsT, typename ActionsT>
class Transition {
public:
  using State = StateT;
  using Event = EventT;
  using Guards = GuardsT;
  using Actions = ActionsT;

  static_assert(validity::isValidGuardTupleType<Guards>(),
                "Guards should be tuple of invocables returning bool");
  static_assert(validity::isValidActionTuple(Actions{}),
                "Actions should be tuple of invocables return void");

private:
  State mSource;
  State mTarget;
  Event mEvent;
  Guards mGuards;
  Actions mActions;

public:
  constexpr Transition(const State &source, const State &target,
                       const Event &event,
                       const Guards &guards = std::tuple<>(),
                       const Actions &actions = std::tuple<>())
      : mSource(source), mTarget(target), mEvent(event), mGuards(guards),
        mActions(actions) {}

  constexpr Transition(const Transition &other) = default;
  constexpr Transition(Transition &&other) = default;
  constexpr Transition &operator=(const Transition &other) = default;
  constexpr Transition &operator=(Transition &&other) = default;
  ~Transition() = default;

  constexpr State source() const { return mSource; }
  constexpr State target() const { return mTarget; }
  constexpr Event event() const { return mEvent; }

  constexpr bool checkGuards() const { return evaluate::checkGuards(mGuards); }
  constexpr void executeActions() { evaluate::executeActions(mActions); }
};

// TODO test!

// TODO StateMachine with Transitions tuple

} // namespace susml::optimized

#endif
