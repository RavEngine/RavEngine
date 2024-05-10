/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "utils/sample_type_conversion.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace vraudio {

namespace {

TEST(MiscMath, Int16ToFloatTest) {
  static const int16 kMinInt16 = -0x7FFF;
  static const int16 kMaxInt16 = 0x7FFF;
  static const float kMinFloat = -1.0f;
  static const float kMaxFloat = 1.0f;

  static const float kFloatRange = kMaxFloat - kMinFloat;
  static const uint16 kInt16Range = kMaxInt16 - kMinInt16;

  for (int16 i = kMinInt16; i < kMaxInt16; i = static_cast<int16>(i + 0xFF)) {
    const float mapped_float =
        static_cast<float>(i) / static_cast<float>(kInt16Range) * kFloatRange;
    float float_result = 0.0f;
    ConvertSampleToFloatFormat(i, &float_result);
    EXPECT_FLOAT_EQ(mapped_float, float_result);
  }
}

TEST(MiscMath, FloatToInt16Test) {
  static const int16 kMinInt16 = -0x7FFF;
  static const int16 kMaxInt16 = 0x7FFF;
  static const float kMinFloat = -1.0f;
  static const float kMaxFloat = 1.0f;
  // NOTE: Int16 maximum is 0x7FFF, NOT 0x8000; see scheme 2) in
  // http://goo.gl/NTRQ1a for background.
  static const float kFloatRange = kMaxFloat - kMinFloat;
  static const uint16 kInt16Range = kMaxInt16 - kMinInt16;

  for (float i = kMinFloat; i < kMaxFloat; i += 0.005f) {
    const int16 mapped_int = static_cast<int16>(i * kInt16Range / kFloatRange);
    int16 int16_result = 0;
    ConvertSampleFromFloatFormat(i, &int16_result);
    EXPECT_EQ(mapped_int, int16_result);
  }
}

TEST(MiscMath, FloatToInt16TestPositiveSaturate) {
  // Maximum positive value is 2^15 - 1
  static const int16 kMaxInt16 = 0x7FFF;
  static const float kMaxFloat = 1.0f;
  int16 int16_result = 0;
  ConvertSampleFromFloatFormat(2 * kMaxFloat, &int16_result);
  EXPECT_EQ(kMaxInt16, int16_result);
}

TEST(MiscMath, FloatToInt16TestNegativeSaturate) {
  static const int16 kMinInt16 = -0x7FFF;
  static const float kMinFloat = -1.0f;
  int16 int16_result = 0;
  ConvertSampleFromFloatFormat(2 * kMinFloat, &int16_result);
  EXPECT_EQ(kMinInt16, int16_result);
}

}  // namespace

}  // namespace vraudio
