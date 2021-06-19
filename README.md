# SUSML - Still Untitled State Machine Library
A small, header-only, finite state machine library, allowing the user to create a state machine with transition guards, transition actions, and trigger events. This library currently requires C++17, it requires the C++ standard library (specifically, `<type_traits>`, `<vector>`, `<algorithm>`. It does not require RTTI. It compiles and should run fine without exceptions, especially if state machines are defined entirely at compile-time.

There are two types of state machines in SUSML.
1. Tuple-based (in the `tuplebased` namespace in `tuplebased.hpp`). Intended for compile-time specification of smaller state machines (say, <30 states), and tries to compete with handcrafted solutions (performance in at least the same order of magnitude as a handcrafted solution). It uses a tuple to store transitions, facilitating Transition types to differ, which in turn enables lambdas to be used directly.
2. Vector-based (in the `vectorbased` namespace in `vectorbased.hpp`). Intended for run-time specification of state machines of any size (though, optimized for smaller ones. If you have more than 1000 transitions you probably want something else). It uses a vector to store transitions, thereby enforcing that each transition has the same type, and thus resolution of guards and actions has to be runtime polymorphic (by default it uses std::function).

# What this will not do

#### State entry/exit actions
I didn't need it so I didn't implement it. However, it should be pretty simple to implement your own trigger function that inspects the machine's state before and after the trigger, and executes whatever actions accordingly.

#### Very large state machines
* on the tuple-based variant, the amount of time it takes to compile goes up pretty fast as you increase the number of transitions, and it will probably fail to compile entirely at some point.
* on the vector-based variant, all the transitions are stored in a vector, which is searched sequentially on any given trigger. That works pretty fast on smaller machines, but on bigger ones it slows down, and eventually results in bad allocations because the vector requires too much contiguous memory.

If you are looking to get a large high-performance state machine, some ideas you might use in rolling your own:
* Split up the transition into its separate parts and store those parts (e.g. have a vector of guards, where element N corresponds to the guard of transition N).
* Place actions and guards into some kind of data structure that can use the source state and/or event as an index, minimizing time spent searching. (e.g. vector-of-vectors, where the first vector has an element for each source state (such that it can be indexed by currentState), s.t. that element contains the target, event, etc of the transition). 

