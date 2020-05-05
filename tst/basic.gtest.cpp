#include "gtest/gtest.h"

#include "susml.hpp"

namespace Guards {
int numUnitGuardCalled = 0;
bool unitGuard() {
  numUnitGuardCalled++;
  return true;
}
} // namespace Guards

namespace Actions {
int numUnitActionCalled = 0;
void unitAction() { numUnitActionCalled++; }
} // namespace Actions

enum class States {
  off,
  on,
  NUM_STATES,
  INITIAL = off,
};

enum class Events { turnOn, turnOff };

using StateMachine = susml::StateMachine<States, Events>;
using Transition = StateMachine::Transition;
using TransitionList = StateMachine::TransitionList;
using TransitionMatrix = StateMachine::TransitionMatrix;

StateMachine createBasicMachine() {
  return StateMachine{TransitionMatrix{
      TransitionList{
          // transitions starting at state off
          {
              Events::turnOn,      // transition in response to turnOn event
              {Guards::unitGuard}, // transition only if unitGuard return true
              {Actions::unitAction,
               Actions::unitAction}, // on transition, call unitAction twice
              States::on             // transition to state on
          },
      },
      TransitionList{
          // transitions starting at state on
          {
              Events::turnOff,     // transition in response to turnOff
                                   // event
              {Guards::unitGuard}, // transition only if unitGuard returns true
              {Actions::unitAction}, // on transition, call unitAction
              States::off            // transition to state off
          },
          {
              Events::turnOn,        // transition in response to turnOn event
              {},                    // transition always
              {Actions::unitAction}, // on transition, call unitAction
              States::on             // transition to state on
          }}}};
}

TEST(BasicTest, GoodWeather) {
  auto m = createBasicMachine();
  Guards::numUnitGuardCalled = 0;
  Actions::numUnitActionCalled = 0;

  EXPECT_EQ(m.getState(), States::INITIAL);
  EXPECT_EQ(m.getState(), States::off);

  m.trigger(Events::turnOn);
  EXPECT_EQ(m.getState(), States::on);
  EXPECT_EQ(Guards::numUnitGuardCalled, 1);
  EXPECT_EQ(Actions::numUnitActionCalled, 2);

  m.trigger(Events::turnOn);
  EXPECT_EQ(m.getState(), States::on);
  EXPECT_EQ(Guards::numUnitGuardCalled, 1);
  EXPECT_EQ(Actions::numUnitActionCalled, 3);

  m.trigger(Events::turnOff);
  EXPECT_EQ(m.getState(), States::off);
  EXPECT_EQ(Guards::numUnitGuardCalled, 2);
  EXPECT_EQ(Actions::numUnitActionCalled, 4);

  m.trigger(Events::turnOff);
  EXPECT_EQ(m.getState(), States::off);
  EXPECT_EQ(Guards::numUnitGuardCalled, 2);
  EXPECT_EQ(Actions::numUnitActionCalled, 4);
}

TEST(BasicTest, BadWeather) {
  // This test does nothing, as there is currently no known reasonable way to
  // make this code fail. That's pretty cool!
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}