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

#ifndef MINIMAL_STATEMACHINE_HPP
#define MINIMAL_STATEMACHINE_HPP

#include "Transition.hpp"
#include <type_traits>

namespace susml::minimal {
template <typename TransitionT, typename TransitionContainerT>
struct StateMachine {
  using Transition = TransitionT;
  using State = typename Transition::State;
  using Event = typename Transition::Event;
  using TransitionContainer = TransitionContainerT;

  static_assert(
      std::is_same<Transition, typename TransitionContainer::value_type>::value,
      "TransitionContainer should be Container of Transition");

  TransitionContainer transitions;
  State currentState;

  constexpr void trigger(const Event &event) {
    for (auto &t : transitions) {
      if (t.source == currentState && t.event == event && t.guard()) {
        t.action();
        currentState = t.target;
        break;
      }
    }
  }
};
} // namespace susml::minimal

#endif
