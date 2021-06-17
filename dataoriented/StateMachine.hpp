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

#ifndef DATAORIENTED_STATEMACHINE_HPP
#define DATAORIENTED_STATEMACHINE_HPP

#include <array>
#include <tuple>
#include <utility>
#include <variant>

#include "Transition.hpp"

namespace susml::dataoriented {

template <typename StateT, typename EventT, typename GuardVariant,
          typename ActionVariant, std::size_t NumTransitions>
struct StateMachine {
  using State = StateT;
  using Event = EventT;

  using Sources = std::array<State, NumTransitions>;
  using Targets = std::array<State, NumTransitions>;
  using Events = std::array<Event, NumTransitions>;
  using Guards = std::array<GuardVariant, NumTransitions>;
  using Actions = std::array<ActionVariant, NumTransitions>;

  Sources sources;
  Targets targets;
  Events events;
  Guards guards;
  Actions actions;

  State currentState;

  constexpr bool checkGuard(GuardVariant &gv) {
    return std::visit([&](auto &g) { return g(); }, gv);
  }

  constexpr void executeAction(ActionVariant &av) {
    std::visit([&](auto &a) { a(); }, av);
  }

  constexpr void trigger(const Event &e) {
    for (std::size_t i = 0; i < NumTransitions; i++) {
      if (sources[i] == currentState && events[i] == e &&
          checkGuard(guards[i])) {
        executeAction(actions[i]);
        currentState = targets[i];
        return;
      }
    }
  }
};

template <typename State, typename... Transitions>
constexpr auto fromTransitions(State initialState,
                               const Transitions &...transitions) {
  constexpr auto NumTransitions = sizeof...(Transitions);

  auto sources = std::array{transitions.source...};
  auto targets = std::array{transitions.target...};
  auto events = std::array{transitions.event...};

  using Event = typename decltype(events)::value_type;

  using GuardVariant = std::variant<typename Transitions::Guard...>;
  using ActionVariant = std::variant<typename Transitions::Action...>;

  auto guards = std::array{GuardVariant{transitions.guard}...};
  auto actions = std::array{ActionVariant{transitions.action}...};

  return StateMachine<State, Event, GuardVariant, ActionVariant,
                      NumTransitions>{sources, targets, events,
                                      guards,  actions, initialState};
}

} // namespace susml::dataoriented

#endif