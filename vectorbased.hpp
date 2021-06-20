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

#ifndef VECTORBASED_HPP
#define VECTORBASED_HPP

#include "common.hpp"
#include <vector>

namespace susml::vectorbased {
template <typename TransitionT>
struct StateMachine {
  using Transition = TransitionT;
  using State      = typename Transition::State;
  using Event      = typename Transition::Event;

  State                   currentState;
  std::vector<Transition> transitions;

  constexpr bool isTransitionTakeable(Transition &t, const Event &event) {
    if constexpr (Transition::HasGuard()) {
      return t.source == currentState && t.event == event && t.guard();
    }
    if constexpr (!Transition::HasGuard()) { return t.source == currentState && t.event == event; }
  }

  constexpr void trigger(const Event &event) {
    for (auto &t : transitions) {
      if (isTransitionTakeable(t, event)) {
        if constexpr (Transition::HasAction()) { t.action(); }
        currentState = t.target;
        break;
      }
    }
  }
};
} // namespace susml::vectorbased

#endif
