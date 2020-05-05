# SUSML - Still Untitled State Machine Library
A small, header-only, finite state machine library, allowing the user to create a state machine with transition guards, transition actions, and trigger events. This library requires C++17 and the C++ standard library (specifically, std::array, std::vector, std::optional). It does not require RTTI. Requirement of exceptions is dubious (compiles without, but I am unsure what the standard library will do in cases where it would normally throw).

## TODO
* Set up build and test scripts of some kind.
* Consider removing inclusion of optional and vector somehow (since this is all in the header we want to include as little as possible).
* Consider making it work with C++11.
* Figure out exception requirement situation.
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
