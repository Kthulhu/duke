#include <gtest/gtest.h>

#include "duke/time/FrameUtils.hpp"
#include <chrono>
#include <stdexcept>

using namespace std;
using namespace std::chrono;

TEST(FrameUtils, frameToTime) {
  EXPECT_EQ(Time(0), frameToTime(0, FrameDuration::PAL));
  EXPECT_EQ(Time(1), frameToTime(25, FrameDuration::PAL));
  EXPECT_EQ(Time(1), frameToTime(24, FrameDuration::FILM));
  EXPECT_EQ(Time(1001, 1000), frameToTime(30, FrameDuration::NTSC));
}

TEST(FrameUtils, timeToFrame) {
  EXPECT_EQ(FrameIndex(0), timeToFrame(Time(0), FrameDuration::PAL));
  EXPECT_EQ(FrameIndex(25), timeToFrame(Time(1), FrameDuration::PAL));
  EXPECT_EQ(FrameIndex(24), timeToFrame(Time(1), FrameDuration::FILM));
  EXPECT_EQ(FrameIndex(30), timeToFrame(Time(1001, 1000), FrameDuration::NTSC));
}

TEST(FrameUtils, timeToMicroseconds) {
  EXPECT_EQ(666667, Time(2, 3).asMicroseconds());
  EXPECT_EQ(1, Time(1, 1000000).asMicroseconds());
  EXPECT_EQ(3600 * 1000000UL, Time(3600).asMicroseconds());
}

void testBackAndForthCalculations(const FrameDuration framerate) {
  for (auto i = 0; i < 10000; ++i) {
    const auto frame = rand();
    ASSERT_EQ(FrameIndex(frame), timeToFrame(frameToTime(frame, framerate), framerate));
  }
}

TEST(FrameUtils, domain) {
  testBackAndForthCalculations(FrameDuration::PAL);
  testBackAndForthCalculations(FrameDuration::NTSC);
  testBackAndForthCalculations(FrameDuration::FILM);
}
