
#ifndef NUM_TRANSITIONS
#define NUM_TRANSITIONS (10)
#endif

#ifndef NUM_TRIGGERS
#define NUM_TRIGGERS (100)
#endif

#ifndef USE_ASSERTS
#define USE_ASSERTS (false)
#endif

#include <iostream>
#include <type_traits>

#include "tuplebased.hpp"

using namespace susml::tuplebased;

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  constexpr auto guards = std::make_tuple();
  auto actions = std::make_tuple([&] { counter += Index + 1; });

  return Transition<int, int, decltype(guards), decltype(actions), USE_ASSERTS>(
      source, target, true, guards, actions);
}

template <std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &,
                               std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::make_tuple(makeTransition<Indices, totalTransitions>(counter)...);
}

int main() {
  size_t counter = 0;

  auto transitions =
      makeTransitions(std::make_index_sequence<NUM_TRANSITIONS>(), counter);
  auto m = StateMachine<int, int, decltype(transitions), USE_ASSERTS>{transitions, 0};

  for (int i = 0; i < NUM_TRIGGERS; i++) {
    m.trigger(true);
  }

  std::cout << std::boolalpha
    << "use asserts: " << USE_ASSERTS << std::endl
    << "num transitions: " << NUM_TRANSITIONS << std::endl
    << "num triggers: " << NUM_TRIGGERS << std::endl;

  return counter;
}