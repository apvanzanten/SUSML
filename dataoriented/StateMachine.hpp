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

#include <functional>
#include <vector>

#include "Transition.hpp"

namespace susml::dataoriented {

template <typename StateT, typename EventT,
          typename GuardT = std::function<bool()>,
          typename ActionT = std::function<void()>>
struct StateMachine {
  using State = StateT;
  using Event = EventT;
  using Guard = GuardT;
  using Action = ActionT;

  using Sources = std::vector<State>;
  using Targets = std::vector<State>;
  using Events = std::vector<Event>;
  using Guards = std::vector<Guard>;
  using Actions = std::vector<Action>;

  static_assert(std::is_invocable<Guard>::value, "Guard should be invocable");
  static_assert(std::is_invocable<Action>::value, "Action should be invocable");
  static_assert(
      std::is_same<typename std::invoke_result<Guard>::type, bool>::value,
      "Guard should return bool.");
  static_assert(
      std::is_same<typename std::invoke_result<Action>::type, void>::value,
      "Action should return void.");

  State currentState;

  Sources sources;
  Targets targets;
  Events events;
  Guards guards;
  Actions actions;

  constexpr StateMachine(State initialState, Sources sources, Targets targets,
                         Events events, Guards guards, Actions actions)
      : currentState(initialState), sources(std::move(sources)),
        targets(std::move(targets)), events(std::move(events)),
        guards(std::move(guards)), actions(std::move(actions)) {}

  constexpr bool isValid() const {
    return (sources.size() == targets.size()) &&
           (targets.size() == events.size()) &&
           (events.size() == guards.size()) &&
           (guards.size() == actions.size());
  }

  constexpr void trigger(const Event &e) {
    for (std::size_t i = 0; i < sources.size(); i++) {
      if (sources[i] == currentState && events[i] == e && guards[i]()) {
        actions[i]();
        currentState = targets[i];
        return;
      }
    }
  }
};

template <typename State, typename Transition>
constexpr auto fromTransitions(State initialState,
                               const std::vector<Transition> &transitions) {

  using Event = typename Transition::Event;
  using Guard = typename Transition::Guard;
  using Action = typename Transition::Action;

  StateMachine<State, Event, Guard, Action> m{
      initialState, {}, {}, {}, {}, {}}; // all vectors default-initialized
  m.sources.reserve(transitions.size());
  m.targets.reserve(transitions.size());
  m.events.reserve(transitions.size());
  m.actions.reserve(transitions.size());
  m.guards.reserve(transitions.size());

  for (const Transition &t : transitions) {
    m.sources.push_back(t.source);
    m.targets.push_back(t.target);
    m.events.push_back(t.event);
    m.actions.push_back(t.action);
    m.guards.push_back(t.guard);
  }

  return m;
}

} // namespace susml::dataoriented

#endif