# SUSML - Still Untitled State Machine Library
A small, header-only, finite state machine library, allowing the user to create a state machine with transition guards, transition actions, and trigger events. This library currently does compile with C++11, but C++17 may be required in the future, it requires the C++ standard library (specifically, std::array, std::vector, and a few functions from <algorithm>). It does not require RTTI. It compiles and should run fine without exceptions, especially if state machines are defined entirely at compile-time (which is the general idea).

## TODO
* ~~Set up build and test scripts of some kind.~~
* Consider removing inclusion of vector somehow (since this is all in the header we want to include as little as possible).
* ~~Consider making it work with C++11.~~
* ~~Figure out exception requirement situation.~~ *revisit if we ever start looking into runtime machine definition.*
* Write tests:
    - non-trivial machines
* Write benchmarks:
    - speed
    - memory usage
    - binary size
* Come up with a better name (maybe)

## Possible future features
* 'Instrumentation' for registering transitions and/or visited states
* Parsing from some common LTS language(s)
* State entry/exit actions
