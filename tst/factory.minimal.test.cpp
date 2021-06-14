
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

#include "minimal/factory.hpp"

#include <algorithm>

using namespace susml::minimal::factory;

enum class State { on, off };
std::ostream &operator<<(std::ostream &stream, State state) {
  stream << "State::" << (state == State::on ? "on" : "off");
  return stream;
}
enum class Event { turnOn, turnOff };
std::ostream &operator<<(std::ostream &stream, Event event) {
  stream << "Event::" << (event == Event::turnOn ? "turnOn" : "turnOff");
  return stream;
}

constexpr bool guardTrue() { return true; }
constexpr bool guardFalse() { return false; }

constexpr void actionA() {}
constexpr void actionB() {}

const PartialTransition<State, Event, bool (*)(), void (*)(),
                            std::vector<bool (*)()>,
                            std::vector<void (*)()>>
    fullyPopulated{State::off,
                   Event::turnOn,
                   {guardTrue, guardFalse},
                   {actionA, actionB},
                   State::on};
  
std::ostream & operator<< (std::ostream &stream, const NoneType &) {
  stream << "None";
  return stream;
}

template <typename State, typename Event, typename Guard,
            typename Action, typename GuardContainer,
            typename ActionContainer>
std::ostream &
operator<<(std::ostream &stream,
           const PartialTransition<State, Event, Guard, Action, GuardContainer, ActionContainer> &t) {
  stream << "From(" << t.source << ").To(" << t.target << ").On(" << t.event <<")";

  if (t.hasGuards()) {
    stream << ".If(...)";
  } else {
    stream << ".If(None)" ;
  }
  if (t.hasActions()) {
    stream << ".Do(...)";
  } else {
    stream << ".Do(None)";
  }
  stream << ";";
  return stream;
}

TEST(PartialTransitionTests, From) {
  const auto t = From(State::off);
  EXPECT_EQ(State::off, t.source);

  const auto tModified = t.From(State::on);
  EXPECT_EQ(State::on, tModified.source);

  const auto fpModified = fullyPopulated.From(State::on);
  EXPECT_EQ(State::on, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);

  EXPECT_TRUE(std::equal(fullyPopulated.guards.begin(),
                         fullyPopulated.guards.end(),
                         fpModified.guards.begin()));

  EXPECT_TRUE(std::equal(fullyPopulated.actions.begin(),
                         fullyPopulated.actions.end(),
                         fpModified.actions.begin()));

  const auto fpUnmodified = fpModified.From(State::off);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, To) {
  const auto t = To(State::off);
  EXPECT_EQ(State::off, t.target);

  const auto tModified = t.To(State::on);
  EXPECT_EQ(State::on, tModified.target);

  const auto fpModified = fullyPopulated.To(State::off);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(State::off, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);

  EXPECT_TRUE(std::equal(fullyPopulated.guards.begin(),
                         fullyPopulated.guards.end(),
                         fpModified.guards.begin()));

  EXPECT_TRUE(std::equal(fullyPopulated.actions.begin(),
                         fullyPopulated.actions.end(),
                         fpModified.actions.begin()));

  const auto fpUnmodified = fpModified.To(State::on);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, On) {
  const auto t = On(Event::turnOn);
  EXPECT_EQ(Event::turnOn, t.event);

  const auto tModified = t.On(Event::turnOff);
  EXPECT_EQ(Event::turnOff, tModified.event);

  const auto fpModified = fullyPopulated.On(Event::turnOff);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(Event::turnOff, fpModified.event);

  EXPECT_TRUE(std::equal(fullyPopulated.guards.begin(),
                         fullyPopulated.guards.end(),
                         fpModified.guards.begin()));

  EXPECT_TRUE(std::equal(fullyPopulated.actions.begin(),
                         fullyPopulated.actions.end(),
                         fpModified.actions.begin()));

  const auto fpUnmodified = fpModified.On(Event::turnOn);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, If) {
  const auto guards = {guardTrue};
  const auto guardsModified = {guardFalse};

  const auto t = If(guards);
  EXPECT_TRUE(std::equal(t.guards.begin(), t.guards.end(), guards.begin()));

  const auto tModified = t.If(guardsModified);
  EXPECT_TRUE(std::equal(tModified.guards.begin(), tModified.guards.end(),
                         guardsModified.begin()));

  const auto fpNoGuards = fullyPopulated.If();
  EXPECT_NE(fullyPopulated, fpNoGuards);
  EXPECT_EQ(fullyPopulated.source, fpNoGuards.source);
  EXPECT_EQ(fullyPopulated.target, fpNoGuards.target);
  EXPECT_EQ(fullyPopulated.event, fpNoGuards.event);

  EXPECT_EQ(0, fpNoGuards.guards.size());

  const auto fpModified = fpNoGuards.If(guardsModified);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_NE(fpNoGuards, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);

  EXPECT_NE(fullyPopulated.guards.size(), fpModified.guards.size());

  EXPECT_TRUE(std::equal(fpModified.guards.begin(), fpModified.guards.end(),
                         guardsModified.begin()));

  const auto fpUnmodified = fpModified.If(fullyPopulated.guards);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, Do) {
  const auto actions = {actionA};
  const auto actionsModified = {actionB};

  const auto t = Do(actions);
  EXPECT_TRUE(std::equal(t.actions.begin(), t.actions.end(), actions.begin()));

  const auto tModified = t.Do(actionsModified);
  EXPECT_TRUE(std::equal(tModified.actions.begin(), tModified.actions.end(),
                         actionsModified.begin()));

  const auto fpNoActions = fullyPopulated.Do();
  EXPECT_NE(fullyPopulated, fpNoActions);
  EXPECT_EQ(fullyPopulated.source, fpNoActions.source);
  EXPECT_EQ(fullyPopulated.target, fpNoActions.target);
  EXPECT_EQ(fullyPopulated.event, fpNoActions.event);

  EXPECT_EQ(0, fpNoActions.actions.size());

  const auto fpModified = fpNoActions.Do(actionsModified);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_NE(fpNoActions, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);

  EXPECT_NE(fullyPopulated.actions.size(), fpModified.actions.size());

  EXPECT_TRUE(std::equal(fpModified.actions.begin(), fpModified.actions.end(),
                         actionsModified.begin()));

  const auto fpUnmodified = fpModified.Do(fullyPopulated.actions);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, FullConstruction) {
  const auto withSource = From(State::off);
  EXPECT_EQ(State::off, withSource.source);

  const auto withTarget = withSource.To(State::on);
  EXPECT_EQ(State::off, withTarget.source);
  EXPECT_EQ(State::on, withTarget.target);

  const auto withEvent = withTarget.On(Event::turnOn);
  EXPECT_EQ(State::off, withEvent.source);
  EXPECT_EQ(State::on, withEvent.target);
  EXPECT_EQ(Event::turnOn, withEvent.event);

  const auto guards = {guardTrue, guardFalse};
  const auto withGuards = withEvent.If(guards);
  EXPECT_EQ(State::off, withGuards.source);
  EXPECT_EQ(State::on, withGuards.target);
  EXPECT_EQ(Event::turnOn, withGuards.event);
  EXPECT_TRUE(
      std::equal(guards.begin(), guards.end(), withGuards.guards.begin()));

  const auto actions = {actionA, actionB};
  const auto withActions = withGuards.Do(actions);
  EXPECT_EQ(State::off, withActions.source);
  EXPECT_EQ(State::on, withActions.target);
  EXPECT_EQ(Event::turnOn, withActions.event);
  EXPECT_TRUE(
      std::equal(guards.begin(), guards.end(), withActions.guards.begin()));
  EXPECT_TRUE(
      std::equal(actions.begin(), actions.end(), withActions.actions.begin()));

  const auto transition = withActions.make();
  EXPECT_EQ(State::off, transition.source);
  EXPECT_EQ(State::on, transition.target);
  EXPECT_EQ(Event::turnOn, transition.event);
  EXPECT_TRUE(
      std::equal(guards.begin(), guards.end(), transition.guards.begin()));
  EXPECT_TRUE(
      std::equal(actions.begin(), actions.end(), transition.actions.begin()));
}

TEST(PartialTransitionTests, makeFullyPopulated) {
  const auto guards = {guardFalse, guardTrue};
  std::cout << "guardFalse: " << reinterpret_cast<void *>(guardFalse)
            << std::endl;
  std::cout << "guardTrue: " << reinterpret_cast<void *>(guardTrue)
            << std::endl;

  const auto partial = From(State::on)
                           .To(State::off)
                           .On(Event::turnOff)
                           .If(guards)
                           .Do({actionB, actionA});
  const auto t = partial.make();
  EXPECT_EQ(partial.source, t.source);
  EXPECT_EQ(partial.target, t.target);
  EXPECT_EQ(partial.event, t.event);

  ASSERT_EQ(partial.guards.size(), t.guards.size());
  EXPECT_TRUE(std::equal(partial.guards.begin(), partial.guards.end(),
                         t.guards.begin()));

  ASSERT_EQ(partial.actions.size(), t.actions.size());
  EXPECT_TRUE(std::equal(partial.actions.begin(), partial.actions.end(),
                         t.actions.begin()));
}

TEST(PartialTransitionTests, makeWithoutGuardsOrActions) {
  const auto partial = From(State::off).To(State::on).On(Event::turnOn);
  const auto t = partial.make();

  EXPECT_EQ(partial.source, t.source);
  EXPECT_EQ(partial.target, t.target);
  EXPECT_EQ(partial.event, t.event);

  ASSERT_FALSE(partial.hasGuards());
  ASSERT_FALSE(partial.hasActions());
  EXPECT_EQ(0, t.guards.size());
  EXPECT_EQ(0, t.actions.size());
}

TEST(PartialTransitionTests, makeWithoutGuards) {
  const auto partial =
      From(State::off).To(State::on).On(Event::turnOn).Do({actionA, actionB});
  const auto t = partial.make();

  EXPECT_EQ(partial.source, t.source);
  EXPECT_EQ(partial.target, t.target);
  EXPECT_EQ(partial.event, t.event);

  ASSERT_FALSE(partial.hasGuards());
  EXPECT_EQ(0, t.guards.size());

  ASSERT_TRUE(partial.hasActions());
  ASSERT_EQ(2, partial.actions.size());
  EXPECT_EQ(2, t.actions.size());

  ASSERT_EQ(partial.actions.size(), t.actions.size());
  EXPECT_TRUE(std::equal(partial.actions.begin(), partial.actions.end(),
                         t.actions.begin()));
}

TEST(PartialTransitionTests, makeWithoutActions) {
  const auto partial = From(State::off)
                           .To(State::on)
                           .On(Event::turnOn)
                           .If({guardTrue, guardFalse});
  const auto t = partial.make();

  EXPECT_EQ(partial.source, t.source);
  EXPECT_EQ(partial.target, t.target);
  EXPECT_EQ(partial.event, t.event);

  ASSERT_FALSE(partial.hasActions());
  EXPECT_EQ(0, t.actions.size());

  ASSERT_EQ(2, partial.guards.size());
  EXPECT_EQ(2, t.guards.size());

  ASSERT_EQ(partial.guards.size(), t.guards.size());
  EXPECT_TRUE(std::equal(partial.guards.begin(), partial.guards.end(),
                         t.guards.begin()));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}