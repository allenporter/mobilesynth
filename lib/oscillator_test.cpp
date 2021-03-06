// oscillator_test.cpp
// Author: Allen Porter <allen@thebends.org>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include "oscillator.h"
#include "gate.h"

using namespace ::ysynth;

namespace {

TEST(Oscillator, OneCyclePerSecond) {
  ConstantSupplier<ControlValue> freq(1.0f);
  Oscillator osc(4, &freq, ClosedGate());
  for (int i = 0; i < 10; i++) {
    EXPECT_FLOAT_EQ(0.0f, osc.GetValue());
    EXPECT_FLOAT_EQ(0.25f, osc.GetValue());
    EXPECT_FLOAT_EQ(0.5f, osc.GetValue());
    EXPECT_FLOAT_EQ(0.75f, osc.GetValue());
  }
}

TEST(Oscillator, TwoCyclesPerSecond) {
  ConstantSupplier<ControlValue> freq(2.0f);
  Oscillator osc(8, &freq, ClosedGate());
  for (int i = 0; i < 10; i++) {
    EXPECT_FLOAT_EQ(0.0f, osc.GetValue());
    EXPECT_FLOAT_EQ(0.25f, osc.GetValue());
    EXPECT_FLOAT_EQ(0.5f, osc.GetValue());
    EXPECT_FLOAT_EQ(0.75f, osc.GetValue());
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  google::InstallFailureSignalHandler();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
