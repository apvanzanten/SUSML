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
set(HEADERS ${PROJECT_SOURCE_DIR}/common.hpp
            ${PROJECT_SOURCE_DIR}/factory.hpp
            ${PROJECT_SOURCE_DIR}/tuplebased.hpp
            ${PROJECT_SOURCE_DIR}/vectorbased.hpp
)

set(CMAKE_CXX_STANDARD 17)

set(WARNINGS -Wall; -Wextra; -pedantic; -Werror;)
set(SANITIZERS -fsanitize=address; -fsanitize=leak; -fsanitize=undefined;)
set(RESTRICTIONS -fno-rtti; -fno-exceptions;)
set(DEBUG_FLAGS -Og; -g;)
set(TEST_FLAGS ${WARNINGS} ${SANITIZERS} ${RESTRICTIONS} ${DEBUG_FLAGS})
set(RELEASE_FLAGS -O3;)
set(BENCHMARK_FLAGS ${WARNINGS} ${RELEASE_FLAGS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

string(CONCAT CLANG_TIDY_CHECKS
        "performance-*,"
        "cppcoreguidelines-*,"
        "bugprone-*,"
        "misc-*,"
        "portabilty-*,"
        "-misc-non-private-member-variables-in-classes," # false-positives on structs
        "modernize-*,"
        "-modernize-use-trailing-return-type," # trailing return types should not be the default
        "readability-*,"
        "-readability-named-parameter," # I use unnamed parameters to indicate they're not used (common in TMP)
        "clang-analyzer-*")

add_custom_target(RunTidy
    COMMAND clang-tidy -p ${CMAKE_BINARY_DIR} ${HEADERS} -checks=${CLANG_TIDY_CHECKS} -header-filter=.*
    DEPENDS ${HEADERS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(benchmark REQUIRED)

function(AddTest TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_DIR}/${TEST_SOURCE})
    target_include_directories(${TEST_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
    target_include_directories(${TEST_NAME} SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(${TEST_NAME} ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    target_compile_options(${TEST_NAME} PUBLIC ${TEST_FLAGS})
    target_link_options(${TEST_NAME} PUBLIC ${SANITIZERS})
    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

function(AddBenchmark BENCH_NAME BENCH_SOURCE)
  add_executable(${BENCH_NAME} ${TEST_DIR}/${BENCH_SOURCE})
  target_include_directories(${BENCH_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
  target_compile_options(${BENCH_NAME} PUBLIC ${BENCHMARK_FLAGS})
  target_link_options(${BENCH_NAME} PUBLIC ${BENCHMARK_FLAGS})
  target_link_libraries(${BENCH_NAME} benchmark)
endfunction()


AddTest(testFactory factory.test.cpp)
AddTest(testVectorBased vectorbased.test.cpp)
AddTest(testTupleBased tuplebased.test.cpp)

AddBenchmark(benchCircleUpTo32 circleUpTo32.bench.cpp)
AddBenchmark(benchCircle64 circle64.bench.cpp)
AddBenchmark(benchEncoderEventBased encoderEventBased.bench.cpp)
AddBenchmark(benchEncoderGuardBased encoderGuardBased.bench.cpp)