// This file is part of Still Untitled State Machine Library (SUSML).
//    Copyright (C) 2021 A.P. van Zanten
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

#include "gtest/gtest.h"

#include "optimized.hpp"

// using susml::optimized::Transition;

using susml::optimized::validity::isGuardTypeValid;
using susml::optimized::validity::isValidGuardTupleType;
using susml::optimized::validity::areGuardTypesAtIndicesValid;

bool trueGuard() { return true; }

void noneAction() {}

TEST(ValidityTests, guards) {
  bool b = true;

  const auto validGuard = [] { return true; };
  const auto validGuardWithCapture = [&] { return b; };
  const auto invalidGuard = [] { return "this is not a bool!"; };
  const auto invalidGuardWithCapture = [&] { b = false; };

  EXPECT_TRUE(isGuardTypeValid<bool()>());
  EXPECT_TRUE(isGuardTypeValid<bool(*)()>());
  EXPECT_TRUE(isGuardTypeValid<decltype(validGuard)>());
  EXPECT_TRUE(isGuardTypeValid<decltype(validGuardWithCapture)>());
  EXPECT_TRUE(isGuardTypeValid<decltype(trueGuard)>());

  EXPECT_FALSE(isGuardTypeValid<int>());
  EXPECT_FALSE(isGuardTypeValid<void()>());
  EXPECT_FALSE(isGuardTypeValid<decltype(invalidGuard)>());

  EXPECT_TRUE((areGuardTypesAtIndicesValid<std::tuple<bool()>>(std::make_index_sequence<1>())));

  EXPECT_TRUE(isValidGuardTupleType<std::tuple<>>());
  EXPECT_TRUE((isValidGuardTupleType<std::tuple<decltype(validGuard)>>()));
  EXPECT_TRUE((isValidGuardTupleType<std::tuple<bool(), bool(*)(), decltype(validGuard), decltype(validGuardWithCapture), decltype(trueGuard)>>()));

  EXPECT_FALSE(isValidGuardTupleType<std::tuple<decltype(invalidGuard)>>());
  EXPECT_FALSE((isValidGuardTupleType<std::tuple<decltype(invalidGuard), decltype(invalidGuardWithCapture)>>()));
}

// TEST(ValidityTests, actions) {
//   bool b = true;

//   EXPECT_TRUE(isValidActionTuple(std::make_tuple()));
//   EXPECT_TRUE(isValidActionTuple(std::make_tuple([&] { b = false; })));
//   EXPECT_TRUE(isValidActionTuple(std::make_tuple([] {}, [&] { b = false; })));

//   EXPECT_FALSE(isValidActionTuple(std::make_tuple([] { return true; })));
//   EXPECT_FALSE(isValidActionTuple(std::make_tuple([&] { b = false; }, 1.0f)));
// }

// TEST(TransitionTests, basic) {
//   enum class State { off, on };
//   enum class Event { turnOn, turnOff };

//   const auto t = Transition{State::off, State::on, Event::turnOn,
//                             std::make_tuple(), std::make_tuple()};

//   EXPECT_EQ(State::off, t.source());
//   EXPECT_EQ(State::on, t.target());
//   EXPECT_EQ(Event::turnOn, t.event());
// }

// TEST(TransitionTests, withGuards) {
//   enum class State { off, on };
//   enum class Event { turnOn, turnOff };

//   bool isReady = false;

//   const auto t = Transition{State::off, State::on, Event::turnOn,
//                             std::make_tuple([&] { return isReady; }),
//                             std::make_tuple([&] { isReady = false; })};
// }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}