
#ifndef NUM_TRANSITIONS
#define NUM_TRANSITIONS (10)
#endif

#ifndef NUM_TRIGGERS
#define NUM_TRIGGERS (100000)
#endif

#ifndef PRINT_INFO
#define PRINT_INFO (false)
#endif

#if PRINT_INFO == (true)
#include <iostream>
#endif

#include <type_traits>

#include "tuplebased.hpp"

using namespace susml::tuplebased;

template <std::size_t Index, std::size_t TotalTransitions>
constexpr auto makeTransition(std::size_t &counter) {
  constexpr auto source = Index;
  constexpr auto target = ((Index + 1) < TotalTransitions) ? Index + 1 : 0;

  constexpr auto guard  = [] { return true; };
  auto           action = [&] {
    counter += Index + 1;
#if PRINT_INFO == (true)
    std::cout << ".";
#endif
  };

  return Transition<int, int, decltype(guard), decltype(action)>(
      source, target, true, guard, action);
}

template <std::size_t... Indices>
constexpr auto makeTransitions(const std::index_sequence<Indices...> &, std::size_t &counter) {
  constexpr auto totalTransitions = sizeof...(Indices);
  return std::make_tuple(makeTransition<Indices, totalTransitions>(counter)...);
}

int main() {
  size_t counter = 0;

  auto transitions = makeTransitions(std::make_index_sequence<NUM_TRANSITIONS>(), counter);
  auto m           = StateMachine<int, int, decltype(transitions)>{transitions, 0};

  for (int i = 0; i < NUM_TRIGGERS; i++) {
    m.trigger(true);
  }

#if PRINT_INFO == (true)
  std::cout << std::endl
            << std::boolalpha << "num transitions: " << NUM_TRANSITIONS << std::endl
            << "num triggers: " << NUM_TRIGGERS << std::endl;
#endif

  return (counter % 7);
}
