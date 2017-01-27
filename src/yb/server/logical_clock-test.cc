// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "yb/server/logical_clock.h"
#include "yb/util/monotime.h"
#include "yb/util/test_util.h"

namespace yb {
namespace server {

class LogicalClockTest : public YBTest {
 public:
  LogicalClockTest()
      : clock_(LogicalClock::CreateStartingAt(HybridTime::kInitialHybridTime)) {
  }

 protected:
  scoped_refptr<LogicalClock> clock_;
};

// Test that two subsequent time reads are monotonically increasing.
TEST_F(LogicalClockTest, TestNow_ValuesIncreaseMonotonically) {
  const HybridTime now1 = clock_->Now();
  const HybridTime now2 = clock_->Now();
  ASSERT_EQ(now1.value() + 1, now2.value());
}

// Tests that the clock gets updated if the incoming value is higher.
TEST_F(LogicalClockTest, TestUpdate_LogicalValueIncreasesByAmount) {
  HybridTime initial = clock_->Now();
  HybridTime future(initial.value() + 10);
  ASSERT_OK(clock_->Update(future));
  HybridTime now = clock_->Now();
  // now should be 1 after future
  ASSERT_EQ(initial.value() + 11, now.value());
}

// Tests that the clock doesn't get updated if the incoming value is lower.
TEST_F(LogicalClockTest, TestUpdate_LogicalValueDoesNotIncrease) {
  HybridTime ht(1);
  // update the clock to 1, the initial value, should do nothing
  ASSERT_OK(clock_->Update(ht));
  HybridTime now = clock_->Now();
  ASSERT_EQ(now.value(), 2);
}

TEST_F(LogicalClockTest, TestWaitUntilAfterIsUnavailable) {
  Status status = clock_->WaitUntilAfter(
      HybridTime(10), MonoTime::Now(MonoTime::FINE));
  ASSERT_TRUE(status.IsServiceUnavailable());
}

TEST_F(LogicalClockTest, TestIsAfter) {
  HybridTime ht1 = clock_->Now();
  ASSERT_TRUE(clock_->IsAfter(ht1));

  // Update the clock in the future, make sure it still
  // handles "IsAfter" properly even when it's running in
  // "logical" mode.
  HybridTime now_increased = HybridTime(1000);
  ASSERT_OK(clock_->Update(now_increased));
  HybridTime ht2 = clock_->Now();

  ASSERT_TRUE(clock_->IsAfter(ht1));
  ASSERT_TRUE(clock_->IsAfter(ht2));
}

}  // namespace server
}  // namespace yb

