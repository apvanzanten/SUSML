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

#include "tuplebased/StateMachine.hpp"

bool trueGuard() { return true; }

void noneAction() {}

TEST(ValidationTests, guards) {
  using susml::tuplebased::validate::isGuardTupleType;
  using susml::tuplebased::validate::isGuardType;

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

TEST(ValidationTests, actions) {
  using susml::tuplebased::validate::isActionTupleType;
  using susml::tuplebased::validate::isActionType;

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

TEST(ValidationTests, transitions) {
  using susml::tuplebased::Transition;
  using susml::tuplebased::validate::isTransitionType;

  enum class State { a, b, c };
  enum class Event { x, y, z };

  auto someGuard = [] { return true; };
  auto someAction = [] {};

  EXPECT_TRUE(
      (isTransitionType<Transition<int, int, std::tuple<>, std::tuple<>>>()));
  EXPECT_TRUE((
      isTransitionType<Transition<State, Event, std::tuple<decltype(someGuard)>,
                                  std::tuple<decltype(someAction)>>>()));

  EXPECT_FALSE(isTransitionType<int>());
}

TEST(ValidationTests, transitionTuples) {
  using susml::tuplebased::Transition;
  using susml::tuplebased::validate::isValidTransitionTupleType;

  EXPECT_TRUE((isValidTransitionTupleType<
               std::tuple<Transition<int, bool>, Transition<int, bool>>, int, bool>()));

  EXPECT_FALSE((isValidTransitionTupleType<
                std::tuple<Transition<int, int>, Transition<int, bool>>, int, bool>()));

  EXPECT_FALSE((isValidTransitionTupleType<
                std::tuple<Transition<int, bool>, Transition<int, int>>, int, bool>()));

  EXPECT_TRUE(
      (isValidTransitionTupleType<std::tuple<
           Transition<int, bool, std::tuple<bool (*)()>>,
           Transition<int, bool, std::tuple<>, std::tuple<void (*)()>>>, int, bool>()));

  EXPECT_FALSE(
      (isValidTransitionTupleType<std::tuple<
           Transition<int, int, std::tuple<bool (*)()>>,
           Transition<int, bool, std::tuple<>, std::tuple<void (*)()>>>, int, bool>()));

  EXPECT_FALSE(
      (isValidTransitionTupleType<std::tuple<
           Transition<int, bool, std::tuple<bool (*)()>>,
           Transition<int, int, std::tuple<>, std::tuple<void (*)()>>>, int, bool>()));
}

TEST(TransitionTests, basic) {
  using susml::tuplebased::Transition;

  enum class State { off, on };
  enum class Event { turnOn, turnOff };

  const auto t = Transition(State::off, State::on, Event::turnOn);

  EXPECT_EQ(State::off, t.source);
  EXPECT_EQ(State::on, t.target);
  EXPECT_EQ(Event::turnOn, t.event);

  EXPECT_TRUE(t.checkGuards());
}

TEST(TransitionTests, basicWithGuard) {
  using susml::tuplebased::Transition;

  bool val = false;
  auto getVal = [&val] { return val; };

  auto t = Transition{0, 0, 0, // 0 integers because we don't really care about
                               // states and events for this test
                      std::make_tuple(getVal)};

  EXPECT_FALSE(t.checkGuards()) << "should return false because val is false";

  val = true;
  EXPECT_TRUE(t.checkGuards()) << "should return true because val is true";
}

TEST(TransitionTests, basicWithAction) {
  using susml::tuplebased::Transition;

  bool val = false;
  auto setVal = [&val] { val = true; };

  auto t = Transition{0, 0, 0, // 0 integers because we don't really care about
                               // states and events for this test
                      std::tuple<>(), std::make_tuple(setVal)};

  ASSERT_FALSE(val);

  t.executeActions(); // should set val to true
  EXPECT_TRUE(val);
}

TEST(TransitionTests, multipleGuards) {
  using susml::tuplebased::Transition;

  bool valA = false;
  int valB = 0;
  std::string valC = "";

  auto getValA = [&valA] { return valA; };
  auto isValBGreaterThanZero = [&valB] { return (valB > 0); };
  auto isValCHello = [&valC] { return (valC == "hello"); };

  auto t =
      Transition{0, 0, 0, // 0 integers because we don't really care about
                          // states and events for this test
                 std::make_tuple(getValA, isValBGreaterThanZero, isValCHello)};

  EXPECT_FALSE(t.checkGuards()) << "should return false because valA is false";

  valA = true;
  EXPECT_FALSE(t.checkGuards())
      << "should return false because valB is not greater than zero";

  valB = 1;
  EXPECT_FALSE(t.checkGuards())
      << "should return false because valC is not equal to \"hello\".";

  valC = "hello";
  EXPECT_TRUE(t.checkGuards());

  valA = false;
  EXPECT_FALSE(t.checkGuards()) << "should return false because valA is false";

  valA = true;
  valB = 0;
  EXPECT_FALSE(t.checkGuards())
      << "should return false because valB is not greater than zero";
}

TEST(TransitionTests, multipleActions) {
  using susml::tuplebased::Transition;

  bool valA = false;
  int valB = 0;
  std::string valC = "";

  auto setValA = [&valA] { valA = true; };
  auto setValBToOne = [&valB] { valB = 1; };
  auto setValCToHello = [&valC] { valC = "hello"; };

  auto t = Transition{0, 0, 0, // 0 integers because we don't really care about
                               // states and events for this test
                      std::tuple<>(),
                      std::make_tuple(setValA, setValBToOne, setValCToHello)};

  ASSERT_FALSE(valA);
  ASSERT_EQ(0, valB);
  ASSERT_EQ("", valC);

  t.executeActions();
  EXPECT_TRUE(valA);
  EXPECT_EQ(1, valB);
  EXPECT_EQ("hello", valC);
}

TEST(TransitionTests, multipleActionsExeuctionOrder) {
  using susml::tuplebased::Transition;

  std::vector<int> out{};

  auto pushZero = [&out] { out.push_back(0); };
  auto pushOne = [&out] { out.push_back(1); };
  auto pushTwo = [&out] { out.push_back(2); };
  auto pushThree = [&out] { out.push_back(3); };

  {
    auto t = Transition{0, 0, 0, // 0 integers because we don't really care
                                 // about states and events for this test
                        std::tuple<>(),
                        std::make_tuple(pushZero, pushOne, pushTwo, pushThree)};

    ASSERT_TRUE(out.empty());

    t.executeActions();
    EXPECT_EQ((std::vector{0, 1, 2, 3}), out);

    out.clear();
  }

  {
    auto t = Transition{0, 0, 0, // 0 integers because we don't really care
                                 // about states and events for this test
                        std::tuple<>(),
                        std::make_tuple(pushOne, pushZero, pushOne, pushThree)};

    ASSERT_TRUE(out.empty());

    t.executeActions();
    EXPECT_EQ((std::vector{1, 0, 1, 3}), out);

    out.clear();
  }
}

TEST(StateMachineTests, basicTransition) {

  enum class State { on, off };
  enum class Event { turnOn, turnOff };

  using Transition = susml::tuplebased::Transition<State, Event>;
  using StateMachine =
      susml::tuplebased::StateMachine<State, Event,
                                      std::tuple<Transition, Transition>>;

  Transition onToOff(State::on, State::off, Event::turnOff);
  Transition offToOn(State::off, State::on, Event::turnOn);

  StateMachine m{std::make_tuple(offToOn, onToOff), State::off};

  ASSERT_EQ(State::off, m.currentState());

  m.trigger(Event::turnOff); // already off, state won't change
  EXPECT_EQ(State::off, m.currentState());

  m.trigger(Event::turnOn);
  EXPECT_EQ(State::on, m.currentState());

  m.trigger(Event::turnOn); // already on, state won't change
  EXPECT_EQ(State::on, m.currentState());

  m.trigger(Event::turnOff);
  EXPECT_EQ(State::off, m.currentState());
}

TEST(StateMachineTests, transitionWithGaurdAndActions) {
  enum class State { on, off };
  enum class Event { turnOn, turnOff };

  using namespace susml;

  bool readyForOn = false;
  bool readyForOff = false;

  std::vector<std::string> reports;

  tuplebased::Transition offToOn(
      State::off, State::on, Event::turnOn,
      std::make_tuple([&] { return readyForOn; }),
      std::make_tuple([&] { reports.push_back("turnOn"); }));
  tuplebased::Transition onToOff(
      State::on, State::off, Event::turnOff,
      std::make_tuple([&] { return readyForOff; }),
      std::make_tuple([&] { reports.push_back("turnOff"); }));

  auto transitions = std::make_tuple(offToOn, onToOff);

  using StateMachine =
      susml::tuplebased::StateMachine<State, Event, decltype(transitions)>;

  StateMachine m{transitions, State::off};

  ASSERT_FALSE(readyForOn);
  ASSERT_FALSE(readyForOff);
  ASSERT_EQ(State::off, m.currentState());

  m.trigger(Event::turnOff); // wrong event
  EXPECT_EQ(State::off, m.currentState());

  m.trigger(Event::turnOn); // right event, but readyForOn is false
  EXPECT_EQ(State::off, m.currentState());

  readyForOn = true;

  m.trigger(Event::turnOn); // right event and readyForOn is true
  EXPECT_EQ(State::on, m.currentState());
  EXPECT_EQ(1, reports.size());
  EXPECT_EQ("turnOn", reports.back());

  m.trigger(Event::turnOff); // right event but readyForOff is false
  EXPECT_EQ(State::on, m.currentState());
  EXPECT_EQ(1, reports.size());

  readyForOff = true;

  m.trigger(Event::turnOff); // right event and readyForOff is true
  EXPECT_EQ(State::off, m.currentState());
  EXPECT_EQ(2, reports.size());
  EXPECT_EQ("turnOff", reports.back());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}