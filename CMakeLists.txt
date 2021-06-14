# This file is part of Still Untitled State Machine Library (SUSML).
#    Copyright (C) 2020 A.P. van Zanten
# SUSML is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# SUSML is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
# You should have received a copy of the GNU Lesser General Public License
# along with SUSML. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.7)
project(susml)

set(TEST_DIR ${PROJECT_SOURCE_DIR}/tst)

set(CMAKE_CXX_STANDARD 17)

set(WARNINGS -Wall; -Wextra; -pedantic; -Werror;)
set(SANITIZERS -fsanitize=address; -fsanitize=leak; -fsanitize=undefined;)
set(RESTRICTIONS -fno-rtti; -fno-exceptions;)
set(DEBUG_FLAGS -Og; -g;)
set(TEST_FLAGS ${WARNINGS} ${SANITIZERS} ${RESTRICTIONS} ${DEBUG_FLAGS})
set(RELEASE_FLAGS -O3;)
set(BENCHMARK_FLAGS ${WARNINGS} ${RESTRICTIONS} ${RELEASE_FLAGS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(testMinimal ${TEST_DIR}/minimal.test.cpp)
target_include_directories(testMinimal PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(testMinimal PUBLIC ${TEST_FLAGS})
target_link_libraries(testMinimal ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_options(testMinimal PUBLIC ${TEST_FLAGS})

add_executable(testMinimalFactory ${TEST_DIR}/factory.minimal.test.cpp)
target_include_directories(testMinimalFactory PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(testMinimalFactory PUBLIC ${TEST_FLAGS})
target_link_libraries(testMinimalFactory ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_options(testMinimalFactory PUBLIC ${TEST_FLAGS})

add_test(MinimalTests testMinimal)
add_test(MinimalTests testMinimalFactory)

add_executable(testTupleBased ${TEST_DIR}/tuplebased.test.cpp)
target_include_directories(testTupleBased PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(testTupleBased ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_compile_options(testTupleBased PUBLIC ${TEST_FLAGS})
target_link_options(testTupleBased PUBLIC ${TEST_FLAGS})
add_test(TupleBasedTests testTupleBased)

find_package(benchmark REQUIRED)

add_executable(benchCircleUpTo16 ${TEST_DIR}/circleUpTo16.bench.cpp)
target_include_directories(benchCircleUpTo16 PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(benchCircleUpTo16 PUBLIC ${BENCHMARK_FLAGS})
target_link_options(benchCircleUpTo16 PUBLIC ${BENCHMARK_FLAGS})
target_link_libraries(benchCircleUpTo16 benchmark)

add_executable(benchCircle32 ${TEST_DIR}/circle32.bench.cpp)
target_include_directories(benchCircle32 PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(benchCircle32 PUBLIC ${BENCHMARK_FLAGS})
target_link_options(benchCircle32 PUBLIC ${BENCHMARK_FLAGS})
target_link_libraries(benchCircle32 benchmark)
