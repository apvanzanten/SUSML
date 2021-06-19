
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

#include "Transition.hpp"
#include "factory.hpp"

#include <algorithm>

using namespace susml::factory;
using susml::NoneType;

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

constexpr void actionA() { return; }
constexpr void actionB() { return; }

const PartialTransition<State, Event, bool (*)(), void (*)()> fullyPopulated{
    State::off, State::on, Event::turnOn, guardTrue, actionA};

std::ostream &operator<<(std::ostream &stream, const NoneType &) {
  stream << "None";
  return stream;
}

template <typename... Types>
std::ostream &operator<<(std::ostream &stream,
                         const PartialTransition<Types...> &t) {
  using P = PartialTransition<Types...>;
  stream << "From(" << t.source << ").To(" << t.target << ").On(" << t.event
         << ")";
  if constexpr (P::HasGuard()) {
    stream << ".If(" << reinterpret_cast<void *>(t.guard) << ")";
  } else if constexpr (!P::HasGuard()) {
    stream << ".If(None)";
  }
  if constexpr (P::HasAction()) {
    stream << ".Do(" << reinterpret_cast<void *>(t.action) << ")";
  } else if constexpr (!P::HasAction()) {
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
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);
  EXPECT_EQ(fullyPopulated.guard, fpModified.guard);
  EXPECT_EQ(fullyPopulated.action, fpModified.action);
  EXPECT_EQ(State::on, fpModified.source);

  const auto fpUnmodified = fpModified.From(State::off);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, To) {
  const auto t = To(State::off);
  EXPECT_EQ(State::off, t.target);
  EXPECT_FALSE(decltype(t)::HasEvent());
  EXPECT_FALSE(decltype(t)::HasGuard());
  EXPECT_FALSE(decltype(t)::HasAction());

  const auto tModified = t.To(State::on);
  EXPECT_EQ(State::on, tModified.target);
  EXPECT_FALSE(decltype(t)::HasEvent());
  EXPECT_FALSE(decltype(t)::HasGuard());
  EXPECT_FALSE(decltype(t)::HasAction());

  const auto fpModified = fullyPopulated.To(State::off);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);
  EXPECT_EQ(fullyPopulated.guard, fpModified.guard);
  EXPECT_EQ(fullyPopulated.action, fpModified.action);
  EXPECT_EQ(State::off, fpModified.target);

  const auto fpUnmodified = fpModified.To(State::on);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, On) {
  const auto t = On(Event::turnOn);
  EXPECT_EQ(Event::turnOn, t.event);
  EXPECT_FALSE(t.HasState());
  EXPECT_FALSE(t.HasGuard());
  EXPECT_FALSE(t.HasAction());

  const auto tModified = t.On(Event::turnOff);
  EXPECT_EQ(Event::turnOff, tModified.event);
  EXPECT_FALSE(t.HasState());
  EXPECT_FALSE(t.HasGuard());
  EXPECT_FALSE(t.HasAction());

  const auto fpModified = fullyPopulated.On(Event::turnOff);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.guard, fpModified.guard);
  EXPECT_EQ(fullyPopulated.action, fpModified.action);
  EXPECT_EQ(Event::turnOff, fpModified.event);

  const auto fpUnmodified = fpModified.On(Event::turnOn);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, If) {
  const auto guard = guardTrue;
  const auto guardModified = guardFalse;

  const auto t = If(guard);
  EXPECT_EQ(guard, t.guard);

  const auto tModified = t.If(guardModified);
  EXPECT_EQ(guardModified, tModified.guard);

  const auto fpNoGuard = fullyPopulated.If();
  EXPECT_NE(fullyPopulated, fpNoGuard);
  EXPECT_EQ(fullyPopulated.source, fpNoGuard.source);
  EXPECT_EQ(fullyPopulated.target, fpNoGuard.target);
  EXPECT_EQ(fullyPopulated.event, fpNoGuard.event);
  EXPECT_EQ(fullyPopulated.action, fpNoGuard.action);
  EXPECT_FALSE(fpNoGuard.HasGuard());

  const auto fpModified = fpNoGuard.If(guardModified);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_NE(fpNoGuard, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);
  EXPECT_EQ(fullyPopulated.action, fpModified.action);
  EXPECT_EQ(guardModified, fpModified.guard);

  const auto fpUnmodified = fpModified.If(fullyPopulated.guard);
  EXPECT_EQ(fullyPopulated, fpUnmodified);
}

TEST(PartialTransitionTests, Do) {
  const auto action = actionA;
  const auto actionModified = actionB;

  const auto t = Do(action);
  EXPECT_EQ(action, t.action);

  const auto tModified = t.Do(actionModified);
  EXPECT_EQ(actionModified, tModified.action);

  const auto fpNoAction = fullyPopulated.Do();
  EXPECT_FALSE(fpNoAction.HasAction());
  EXPECT_NE(fullyPopulated, fpNoAction);
  EXPECT_EQ(fullyPopulated.source, fpNoAction.source);
  EXPECT_EQ(fullyPopulated.target, fpNoAction.target);
  EXPECT_EQ(fullyPopulated.event, fpNoAction.event);
  EXPECT_EQ(fullyPopulated.guard, fpNoAction.guard);

  const auto fpModified = fpNoAction.Do(actionModified);
  EXPECT_NE(fullyPopulated, fpModified);
  EXPECT_NE(fpNoAction, fpModified);
  EXPECT_EQ(fullyPopulated.source, fpModified.source);
  EXPECT_EQ(fullyPopulated.target, fpModified.target);
  EXPECT_EQ(fullyPopulated.event, fpModified.event);
  EXPECT_EQ(fullyPopulated.guard, fpNoAction.guard);
  EXPECT_EQ(actionModified, fpModified.action);

  const auto fpUnmodified = fpModified.Do(fullyPopulated.action);
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

  const auto withGuard = withEvent.If(guardFalse);
  EXPECT_EQ(State::off, withGuard.source);
  EXPECT_EQ(State::on, withGuard.target);
  EXPECT_EQ(Event::turnOn, withGuard.event);
  EXPECT_EQ(guardFalse, withGuard.guard);

  const auto withAction = withGuard.Do(actionA);
  EXPECT_EQ(State::off, withAction.source);
  EXPECT_EQ(State::on, withAction.target);
  EXPECT_EQ(Event::turnOn, withAction.event);
  EXPECT_EQ(guardFalse, withAction.guard);
  EXPECT_EQ(actionA, withAction.action);

  const auto transition = withAction.make();
  EXPECT_EQ(State::off, transition.source);
  EXPECT_EQ(State::on, transition.target);
  EXPECT_EQ(Event::turnOn, transition.event);
  EXPECT_EQ(guardFalse, transition.guard);
  EXPECT_EQ(actionA, transition.action);
}

TEST(PartialTransitionTests, makeFullyPopulated) {
  const auto p = From(State::on)
                     .To(State::off)
                     .On(Event::turnOff)
                     .If(guardFalse)
                     .Do(actionB);
  const auto t = p.make();
  ASSERT_EQ(State::on, p.source);
  ASSERT_EQ(State::off, p.target);
  ASSERT_EQ(Event::turnOff, p.event);
  ASSERT_EQ(guardFalse, p.guard);
  ASSERT_EQ(actionB, p.action);

  EXPECT_EQ(State::on, t.source);
  EXPECT_EQ(State::off, t.target);
  EXPECT_EQ(Event::turnOff, t.event);
  EXPECT_EQ(guardFalse, t.guard);
  EXPECT_EQ(actionB, t.action);
}

TEST(PartialTransitionTests, makeWithoutGuardsOrActions) {
  const auto partial = From(State::off).To(State::on).On(Event::turnOn);

  ASSERT_FALSE(partial.HasGuard());
  ASSERT_FALSE(partial.HasAction());

  const auto t = partial.make();

  EXPECT_EQ(partial.source, t.source);
  EXPECT_EQ(partial.target, t.target);
  EXPECT_EQ(partial.event, t.event);
  EXPECT_FALSE(t.HasGuard());
  EXPECT_FALSE(t.HasAction());
}

TEST(PartialTransitionTests, makeWithoutGuards) {
  const auto p = From(State::off).To(State::on).On(Event::turnOn).Do(actionA);
  ASSERT_TRUE(p.HasAction());
  ASSERT_FALSE(p.HasGuard());
  ASSERT_EQ(State::off, p.source);
  ASSERT_EQ(State::on, p.target);
  ASSERT_EQ(Event::turnOn, p.event);
  ASSERT_EQ(actionA, p.action);

  const auto t = p.make();
  EXPECT_TRUE(t.HasAction());
  EXPECT_FALSE(t.HasGuard());

  EXPECT_EQ(State::off, t.source);
  EXPECT_EQ(State::on, t.target);
  EXPECT_EQ(Event::turnOn, t.event);
  EXPECT_EQ(actionA, t.action);
}

TEST(PartialTransitionTests, makeWithoutActions) {
  const auto p =
      From(State::off).To(State::on).On(Event::turnOn).If(guardFalse);
  ASSERT_FALSE(p.HasAction());
  ASSERT_TRUE(p.HasGuard());
  ASSERT_EQ(State::off, p.source);
  ASSERT_EQ(State::on, p.target);
  ASSERT_EQ(Event::turnOn, p.event);
  ASSERT_EQ(guardFalse, p.guard);

  const auto t = p.make();
  EXPECT_FALSE(t.HasAction());
  EXPECT_TRUE(t.HasGuard());
  EXPECT_EQ(State::off, t.source);
  EXPECT_EQ(State::on, t.target);
  EXPECT_EQ(Event::turnOn, t.event);
  EXPECT_EQ(guardFalse, t.guard);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}