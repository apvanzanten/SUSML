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

bool trueGuard() { return true; }

void noneAction() {}

TEST(ValidityTests, guards) {
  using susml::optimized::validate::isGuardTupleType;
  using susml::optimized::validate::isGuardType;

  bool b = true;

  const auto validGuard = [] { return true; };
  const auto validGuardWithCapture = [&] { return b; };
  const auto invalidGuard = [] { return "this is not a bool!"; };
  const auto invalidGuardWithCapture = [&] { b = false; };

  EXPECT_TRUE(isGuardType<bool()>());
  EXPECT_TRUE(isGuardType<bool (*)()>());
  EXPECT_TRUE(isGuardType<std::function<bool()>>());
  EXPECT_TRUE(isGuardType<decltype(validGuard)>());
  EXPECT_TRUE(isGuardType<decltype(validGuardWithCapture)>());
  EXPECT_TRUE(isGuardType<decltype(trueGuard)>());

  EXPECT_FALSE(isGuardType<bool>());
  EXPECT_FALSE(isGuardType<void()>());
  EXPECT_FALSE(isGuardType<decltype(invalidGuard)>());
  EXPECT_FALSE(isGuardType<decltype(invalidGuardWithCapture)>());

  EXPECT_TRUE(isGuardTupleType<std::tuple<>>());
  EXPECT_TRUE((isGuardTupleType<std::tuple<decltype(validGuard)>>()));
  EXPECT_TRUE(
      (isGuardTupleType<
          std::tuple<bool(), bool (*)(), decltype(validGuard),
                     decltype(validGuardWithCapture), decltype(trueGuard)>>()));

  EXPECT_FALSE(isGuardTupleType<std::tuple<decltype(invalidGuard)>>());
  EXPECT_FALSE(
      (isGuardTupleType<std::tuple<decltype(invalidGuard),
                                   decltype(invalidGuardWithCapture)>>()));
}

TEST(ValidityTests, actions) {
  using susml::optimized::validate::isActionTupleType;
  using susml::optimized::validate::isActionType;

  bool b = true;

  const auto validAction = [] {};
  const auto validActionWithCapture = [&] { b = false; };
  const auto invalidAction = [] { return false; };
  const auto invalidActionWithCapture = [&] { return true; };

  EXPECT_TRUE(isActionType<void()>());
  EXPECT_TRUE(isActionType<void (*)()>());
  EXPECT_TRUE(isActionType<std::function<void()>>());
  EXPECT_TRUE(isActionType<decltype(validAction)>());
  EXPECT_TRUE(isActionType<decltype(validActionWithCapture)>());

  EXPECT_FALSE(isActionType<void>());
  EXPECT_FALSE(isActionType<bool()>());
  EXPECT_FALSE(isActionType<decltype(invalidAction)>());
  EXPECT_FALSE(isActionType<decltype(invalidActionWithCapture)>());

  EXPECT_TRUE((isActionTupleType<std::tuple<>>()));
  EXPECT_TRUE((isActionTupleType<std::tuple<decltype(validAction)>>()));
  EXPECT_TRUE(
      (isActionTupleType<std::tuple<void(), void (*)(), decltype(validAction),
                                    decltype(validActionWithCapture)>>()));

  EXPECT_FALSE(isActionTupleType<std::tuple<decltype(invalidAction)>>());
  EXPECT_FALSE(
      (isActionTupleType<std::tuple<decltype(validAction),
                                    decltype(invalidActionWithCapture)>>()));
}

TEST(TransitionTests, basic) {
  using susml::optimized::Transition;

  enum class State { off, on };
  enum class Event { turnOn, turnOff };

  const auto t = Transition(State::off, State::on, Event::turnOn);

  EXPECT_EQ(State::off, t.source);
  EXPECT_EQ(State::on, t.target);
  EXPECT_EQ(Event::turnOn, t.event);
}

TEST(TransitionTests, basicWithGuardAndAction) {
  using susml::optimized::Transition;

  enum class State { off, on };
  enum class Event { turnOn, turnOff };

  bool isReady = false;
  auto checkIsReady = [&isReady] { return isReady; };
  auto unsetReady = [&isReady] { isReady = false; };

  auto t =
      Transition{State::off, State::on, Event::turnOn,
                 std::make_tuple(checkIsReady), std::make_tuple(unsetReady)};

  EXPECT_EQ(State::off, t.source);
  EXPECT_EQ(State::on, t.target);
  EXPECT_EQ(Event::turnOn, t.event); 

  EXPECT_FALSE(t.checkGuards()) << "should return false because isReady is false";

  isReady = true;
  EXPECT_TRUE(t.checkGuards()) << "should return true because isReady is true";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}