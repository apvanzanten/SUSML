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

#ifndef TRANSITION_HPP
#define TRANSITION_HPP

#include <type_traits>

namespace susml {

template <typename StateT, typename EventT, typename GuardT, typename ActionT>
struct Transition {
  using State = StateT;
  using Event = EventT;
  using Guard = GuardT;
  using Action = ActionT;

  static_assert(std::is_invocable<Guard>::value, "Guard should be invocable");
  static_assert(std::is_invocable<Action>::value, "Action should be invocable");
  static_assert(
      std::is_same<typename std::invoke_result<Guard>::type, bool>::value,
      "Guard should return bool.");
  static_assert(
      std::is_same<typename std::invoke_result<Action>::type, void>::value,
      "Action should return void.");

  State source;
  State target;
  Event event;
  Guard guard;
  Action action;

  constexpr Transition(const State &s, const State &t, const Event &e,
                       const Guard &g, const Action &a)
      : source(s), target(t), event(e), guard(g), action(a) {}
};

} // namespace susml

#endif
