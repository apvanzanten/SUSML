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

set(WARNINGS "-Wall -Wextra -pedantic -Werror")
set(SANITIZERS "-fsanitize=address -fsanitize=leak -fsanitize=undefined")
set(RESTRICTIONS "-fno-rtti -fno-exceptions")
set(DEBUG_FLAGS "-Og -g")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${SANITIZERS} ${RESTRICTIONS} ${DEBUG_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(basicTest ${TEST_DIR}/basic.test.cpp)
target_include_directories(basicTest PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(basicTest ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(allBasicTests basicTest)

add_executable(compositeTest ${TEST_DIR}/composite.test.cpp)
target_include_directories(compositeTest PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(compositeTest ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(allCompositeTests compositeTest)

add_executable(factoryTest ${TEST_DIR}/factory.test.cpp)
target_include_directories(factoryTest PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(factoryTest ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(allFactoryTests factoryTest)

add_executable(tuplebasedTest ${TEST_DIR}/tuplebased.test.cpp)
target_include_directories(tuplebasedTest PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(tuplebasedTest ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(allTupleBasedTests tuplebasedTest)

find_package(benchmark REQUIRED)

add_executable(circleUpTo16Bench ${TEST_DIR}/circleUpTo16.bench.cpp)
target_include_directories(circleUpTo16Bench PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(circleUpTo16Bench PUBLIC -O3)
target_link_libraries(circleUpTo16Bench benchmark)

add_executable(circle32Bench ${TEST_DIR}/circle32.bench.cpp)
target_include_directories(circle32Bench PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(circle32Bench PUBLIC -O3)
target_link_libraries(circle32Bench benchmark)
