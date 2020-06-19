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

#ifndef SUSML_HPP
#define SUSML_HPP

#include <array>
#include <functional>
#include <type_traits>
#include <vector>

namespace susml {

template <typename State, // Should be enum with items for states + NUM_STATES
                          // and INITIAL
          typename Event, // Should be enum with items for events
          typename Guard = std::function<bool()>,
          typename Action = std::function<void()>>
class StateMachine {
public:
  static_assert(std::is_enum<State>::value, "State should be enum");
  static_assert(std::is_enum<Event>::value, "Event should be enum");
  static_assert(std::is_invocable<Guard>::value, "Guard should be invocable");
  static_assert(std::is_invocable<Action>::value, "Action should be invocable");
  static_assert(
      std::is_same<typename std::invoke_result<Guard>::type, bool>::value,
      "Guard should return bool.");
  static_assert(
      std::is_same<typename std::invoke_result<Action>::type, void>::value,
      "Action should return void.");

  struct Transition {
    Event event;
    std::vector<Guard> guards;
    std::vector<Action> actions;
    State target;

    constexpr bool checkGuards() const {
      return std::all_of(guards.begin(), guards.end(),
                         [](const Guard &g) { return g(); });
    }

    constexpr void executeActions() const {
      for (const auto &a : actions) {
        a();
      }
    }
  };

  using TransitionList = std::vector<Transition>;
  using TransitionMatrix =
      std::array<TransitionList, static_cast<std::size_t>(State::NUM_STATES)>;

private:
  const TransitionMatrix transitionMatrix;
  State current_state = State::INITIAL;

  constexpr std::size_t getCurrentStateIndex() const {
    return static_cast<std::size_t>(current_state);
  }

  constexpr const TransitionList &getTransitionsFromCurrentState() const {
    return transitionMatrix[getCurrentStateIndex()];
  }

public:
  constexpr StateMachine(TransitionMatrix transitionMatrix)
      : transitionMatrix(transitionMatrix) {}

  constexpr State getState() const { return current_state; }

  void trigger(Event event) {
    const auto tList = getTransitionsFromCurrentState();
    const auto it =
        std::find_if(tList.begin(), tList.end(), [&event](const Transition &t) {
          return t.event == event && t.checkGuards();
        });

    if (it != tList.end()) {
      const auto &t = *it;
      t.executeActions();
      current_state = t.target;
    }
  }
};
} // namespace susml

#endif
