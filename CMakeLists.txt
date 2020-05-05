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

set(CMAKE_CXX_STANDARD 11)

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

add_executable(basicGTest ${TEST_DIR}/basic.gtest.cpp)
target_include_directories(basicGTest PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(basicGTest ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(allBasicGTests basicGTest)
