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

#include "ambisonics/ambisonic_codec_impl.h"

#include <cmath>
#include <memory>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

const int kCubeNumVertices = 8;
const float kCubeVertices[kCubeNumVertices][3] = {
    {-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}};

// Generates a third-order B-format encoder matrix.
AmbisonicCodecImpl<>::EncoderMatrix GenerateThirdOrderBFormatEncoderMatrix(
    const SphericalAngle& spherical_angle) {
  const size_t kNumSphericalHarmonics = GetNumPeriphonicComponents(3);
  const size_t kNumAngles = 1;
  AmbisonicCodecImpl<>::EncoderMatrix encoder_matrix(kNumSphericalHarmonics,
                                                     kNumAngles);
  const float azimuth_rad = spherical_angle.azimuth();
  const float elevation_rad = spherical_angle.elevation();
  encoder_matrix << 1.0f, std::cos(elevation_rad) * std::sin(azimuth_rad),
      std::sin(elevation_rad), std::cos(elevation_rad) * std::cos(azimuth_rad),
      kSqrtThree / 2.0f * std::cos(elevation_rad) * std::cos(elevation_rad) *
          std::sin(2.0f * azimuth_rad),
      kSqrtThree / 2.0f * std::sin(2.0f * elevation_rad) *
          std::sin(azimuth_rad),
      0.5f * (3.0f * std::sin(elevation_rad) * std::sin(elevation_rad) - 1.0f),
      kSqrtThree / 2.0f * std::sin(2.0f * elevation_rad) *
          std::cos(azimuth_rad),
      kSqrtThree / 2.0f * std::cos(elevation_rad) * std::cos(elevation_rad) *
          std::cos(2.0f * azimuth_rad),
      std::sqrt(5.0f / 8.0f) * IntegerPow(std::cos(elevation_rad), 3) *
          std::sin(3.0f * azimuth_rad),
      std::sqrt(15.0f) / 2.0f * std::sin(elevation_rad) *
          std::cos(elevation_rad) * std::cos(elevation_rad) *
          std::sin(2.0f * azimuth_rad),
      std::sqrt(3.0f / 8.0f) * std::cos(elevation_rad) *
          (5.0f * std::sin(elevation_rad) * std::sin(elevation_rad) - 1.0f) *
          std::sin(azimuth_rad),
      0.5f * std::sin(elevation_rad) *
          (5.0f * std::sin(elevation_rad) * std::sin(elevation_rad) - 3.0f),
      std::sqrt(3.0f / 8.0f) * std::cos(elevation_rad) *
          (5.0f * std::sin(elevation_rad) * std::sin(elevation_rad) - 1.0f) *
          std::cos(azimuth_rad),
      std::sqrt(15.0f) / 2.0f * std::sin(elevation_rad) *
          std::cos(elevation_rad) * std::cos(elevation_rad) *
          std::cos(2.0f * azimuth_rad),
      std::sqrt(5.0f / 8.0f) * IntegerPow(std::cos(elevation_rad), 3) *
          std::cos(3.0f * azimuth_rad);
  return encoder_matrix;
}

class AmbisonicCodecTest : public ::testing::Test {
 protected:
  AmbisonicCodecTest() {
    for (int i = 0; i < kCubeNumVertices; i++) {
      const float azimuth_rad =
          atan2f(kCubeVertices[i][2], kCubeVertices[i][0]);
      const float elevation_rad =
          std::asin(kCubeVertices[i][1] /
                    std::sqrt(kCubeVertices[i][0] * kCubeVertices[i][0] +
                              kCubeVertices[i][1] * kCubeVertices[i][1] +
                              kCubeVertices[i][2] * kCubeVertices[i][2]));

      cube_angles_.push_back(SphericalAngle(azimuth_rad, elevation_rad));
    }

    codec_first_order_cube_ = std::unique_ptr<AmbisonicCodecImpl<>>(
        new AmbisonicCodecImpl<>(1, cube_angles_));
  }

  std::unique_ptr<AmbisonicCodecImpl<>> codec_first_order_cube_;
  std::vector<SphericalAngle> cube_angles_;
};

// Tests that encoder and decoder matrices are ~inverses of each other.
TEST_F(AmbisonicCodecTest, EncoderDecoderIdentity) {
  const auto encoder_matrix = codec_first_order_cube_->GetEncoderMatrix();
  const auto decoder_matrix = codec_first_order_cube_->GetDecoderMatrix();

  const auto should_be_identity = encoder_matrix * decoder_matrix;
  EXPECT_TRUE(should_be_identity.isApprox(
      decltype(should_be_identity)::Identity(should_be_identity.rows(),
                                             should_be_identity.cols()),
      kEpsilonFloat))
      << "Matrix should be within " << kEpsilonFloat
      << " of an identity matrix: \n"
      << should_be_identity;
}

// Tests that Encode and Decode are ~inverse operations.
TEST_F(AmbisonicCodecTest, EncodeDecodeIsInverse) {
  AmbisonicCodecImpl<>::DecodedVector unencoded_vector(8);
  unencoded_vector << 1, 2, 3, 4, 5, 6, 7, 8;
  AmbisonicCodecImpl<>::EncodedVector encoded_vector(4);
  codec_first_order_cube_->Encode(unencoded_vector, encoded_vector);
  AmbisonicCodecImpl<>::DecodedVector decoded_vector(8);
  codec_first_order_cube_->Decode(encoded_vector, decoded_vector);

  EXPECT_TRUE(unencoded_vector.isApprox(decoded_vector, kEpsilonFloat))
      << "Decoded vector should be within " << kEpsilonFloat << " of: \n"
      << unencoded_vector;
}

// Tests that EncodeBuffer and Decode (over a buffer) are ~inverse operations.
TEST_F(AmbisonicCodecTest, EncodeBufferDecodeVectorsIsInverse) {
  const int kNumElements = 256;
  const int kNumSphericalHarmonics = GetNumPeriphonicComponentsStatic<1>::value;

  AudioBuffer unencoded_buffer(kCubeNumVertices, kNumElements);
  // Produce a buffer of successive numbers between -1 and 1.
  for (int element = 0; element < kNumElements; ++element) {
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      unencoded_buffer[angle][element] =
          static_cast<float>(element * kCubeNumVertices + angle) /
              (0.5f * kNumElements * kCubeNumVertices) -
          1.0f;
    }
  }

  AudioBuffer encoded_buffer(kNumSphericalHarmonics, kNumElements);
  codec_first_order_cube_->EncodeBuffer(unencoded_buffer, &encoded_buffer);

  // Verify the encoded buffer and decoded vectors are ~identical.
  for (int element = 0; element < kNumElements; ++element) {
    AmbisonicCodecImpl<>::EncodedVector encoded_vector(kNumSphericalHarmonics);
    for (int harmonic = 0; harmonic < kNumSphericalHarmonics; ++harmonic) {
      encoded_vector[harmonic] = encoded_buffer[harmonic][element];
    }
    AmbisonicCodecImpl<>::DecodedVector decoded_vector(kCubeNumVertices);
    codec_first_order_cube_->Decode(encoded_vector, decoded_vector);
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      EXPECT_NEAR(unencoded_buffer[angle][element], decoded_vector[angle],
                  kEpsilonFloat);
    }
  }
}

// Tests that Encode (over a buffer) and DecodeBuffer are ~inverse operations.
TEST_F(AmbisonicCodecTest, EncodeVectorsDecodeBufferIsInverse) {
  const int kNumElements = 256;
  const int kNumSphericalHarmonics = GetNumPeriphonicComponentsStatic<1>::value;

  AudioBuffer unencoded_buffer(kCubeNumVertices, kNumElements);
  // Produce a buffer of successive numbers between -1 and 1.
  for (int element = 0; element < kNumElements; ++element) {
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      unencoded_buffer[angle][element] =
          static_cast<float>(element * kCubeNumVertices + angle) /
              (0.5f * kNumElements * kCubeNumVertices) -
          1.0f;
    }
  }

  AudioBuffer encoded_buffer(kNumSphericalHarmonics, kNumElements);
  // Produce the encoded version of unencoded_buffer.
  for (int element = 0; element < kNumElements; ++element) {
    AmbisonicCodecImpl<>::DecodedVector unencoded_vector(kCubeNumVertices);
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      unencoded_vector[angle] = unencoded_buffer[angle][element];
    }
    AmbisonicCodecImpl<>::EncodedVector encoded_vector(kNumSphericalHarmonics);
    codec_first_order_cube_->Encode(unencoded_vector, encoded_vector);
    for (int harmonic = 0; harmonic < kNumSphericalHarmonics; ++harmonic) {
      encoded_buffer[harmonic][element] = encoded_vector[harmonic];
    }
  }

  AudioBuffer decoded_buffer(kCubeNumVertices, kNumElements);
  codec_first_order_cube_->DecodeBuffer(encoded_buffer, &decoded_buffer);
  // Verify the decoded buffer and unencoded buffer are ~identical.
  for (int element = 0; element < kNumElements; ++element) {
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      EXPECT_NEAR(unencoded_buffer[angle][element],
                  decoded_buffer[angle][element], kEpsilonFloat);
    }
  }
}

// Tests that EncodeBuffer and DecodeBuffer are ~inverse operations.
TEST_F(AmbisonicCodecTest, EncodeBufferDecodeBufferIsInverse) {
  const int kNumElements = 256;
  const int kNumSphericalHarmonics = GetNumPeriphonicComponentsStatic<1>::value;

  AudioBuffer unencoded_buffer(kCubeNumVertices, kNumElements);
  // Produce a buffer of successive numbers between -1 and 1.
  for (int element = 0; element < kNumElements; ++element) {
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      unencoded_buffer[angle][element] =
          static_cast<float>(element * kCubeNumVertices + angle) /
              (0.5f * kNumElements * kCubeNumVertices) -
          1.0f;
    }
  }



  AudioBuffer encoded_buffer(kNumSphericalHarmonics, kNumElements);
  // Produce the encoded version of unencoded_buffer.
  for (int element = 0; element < kNumElements; ++element) {
    AmbisonicCodecImpl<>::DecodedVector unencoded_vector(kCubeNumVertices);
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      unencoded_vector[angle] = unencoded_buffer[angle][element];
    }
    AmbisonicCodecImpl<>::EncodedVector encoded_vector(kNumSphericalHarmonics);
    codec_first_order_cube_->Encode(unencoded_vector, encoded_vector);
    for (int harmonic = 0; harmonic < kNumSphericalHarmonics; ++harmonic) {
      encoded_buffer[harmonic][element] = encoded_vector[harmonic];
    }
  }

  AudioBuffer decoded_buffer(kCubeNumVertices, kNumElements);
  codec_first_order_cube_->DecodeBuffer(encoded_buffer, &decoded_buffer);
  // Verify the decoded buffer and unencoded buffer are ~identical.
  for (int element = 0; element < kNumElements; ++element) {
    for (int angle = 0; angle < kCubeNumVertices; ++angle) {
      EXPECT_NEAR(unencoded_buffer[angle][element],
                  decoded_buffer[angle][element], kEpsilonFloat);
    }
  }
}

// Tests that third-order encoding coefficients produced by Codec are correct.
TEST_F(AmbisonicCodecTest, ThirdOrderEncodingCoefficients) {
  const int kNumSphericalHarmonics = GetNumPeriphonicComponentsStatic<3>::value;

  const float kAzimuthStart = -180.0f;
  const float kAzimuthStop = 180.0f;
  const float kAzimuthStep = 10.0f;
  for (float azimuth = kAzimuthStart; azimuth <= kAzimuthStop;
       azimuth += kAzimuthStep) {
    const float kElevationStart = -90.0f;
    const float kElevationStop = 90.0f;
    const float kElevationStep = 10.0f;
    for (float elevation = kElevationStart; elevation <= kElevationStop;
         elevation += kElevationStep) {
      const int kAmbisonicOrder = 3;

      const SphericalAngle kSphericalAngle =
          SphericalAngle::FromDegrees(azimuth, elevation);
      AmbisonicCodecImpl<> mono_codec(kAmbisonicOrder, {kSphericalAngle});

      AmbisonicCodecImpl<>::DecodedVector kUnencodedVector(1);
      kUnencodedVector << 0.12345f;
      AmbisonicCodecImpl<>::EncodedVector codec_encoded_vector(
          kNumSphericalHarmonics);
      mono_codec.Encode(kUnencodedVector, codec_encoded_vector);
      const AmbisonicCodecImpl<>::EncoderMatrix
          expected_b_format_encoder_matrix =
              GenerateThirdOrderBFormatEncoderMatrix(kSphericalAngle);
      const AmbisonicCodecImpl<>::EncodedVector expected_encoded_vector =
          static_cast<AmbisonicCodecImpl<>::EncodedVector>(
              expected_b_format_encoder_matrix * kUnencodedVector);
      ASSERT_TRUE(
          codec_encoded_vector.isApprox(expected_encoded_vector, kEpsilonFloat))
          << "Codec encoded vector: \n"
          << codec_encoded_vector << "\n should be within " << kEpsilonFloat
          << " of expected: \n"
          << expected_encoded_vector;
    }
  }
}

// Tests that Sphere16 decoding matrix (ACN/SN3D) coefficients produced by
// Codec
// are correct.
TEST_F(AmbisonicCodecTest, Sphere16DecoderCoefficients) {
  // Expected decoder coefficients for the Sphere16 setup (ACN/SN3D) obtained
  // using //matlab/ambix/ambdecodematrix.m.
  const std::vector<float> kExpectedDecoderCoefficients{
      0.0625000000000001f,    0.0625000000000000f,    0.0625000000000000f,
      0.0625000000000000f,    0.0625000000000000f,    0.0625000000000000f,
      0.0625000000000000f,    0.0625000000000000f,    0.0625000000000000f,
      0.0625000000000000f,    0.0625000000000000f,    0.0625000000000000f,
      0.0625000000000000f,    0.0625000000000000f,    0.0625000000000000f,
      0.0625000000000000f,    -1.94257951433956e-17f, 0.128971506904330f,
      0.182393254223798f,     0.128971506904330f,     1.73262494006158e-16f,
      -0.128971506904330f,    -0.182393254223798f,    -0.128971506904330f,
      -2.79581233842125e-16f, 0.116817928199481f,     -1.55203460683255e-16f,
      -0.116817928199482f,    0.0826027491940168f,    0.0826027491940169f,
      -0.0826027491940164f,   -0.0826027491940165f,   -0.0440164745323702f,
      0.0440164745323702f,    -0.0440164745323703f,   0.0440164745323702f,
      -0.0440164745323703f,   0.0440164745323702f,    -0.0440164745323702f,
      0.0440164745323703f,    0.146497161016369f,     0.146497161016369f,
      0.146497161016369f,     0.146497161016369f,     -0.146497161016369f,
      -0.146497161016369f,    -0.146497161016369f,    -0.146497161016369f,
      0.182393254223799f,     0.128971506904330f,     7.26758979552257e-17f,
      -0.128971506904330f,    -0.182393254223798f,    -0.128971506904330f,
      -5.02371557522155e-17f, 0.128971506904330f,     0.116817928199482f,
      6.83999538850465e-17f,  -0.116817928199482f,    -3.47728677633385e-17f,
      0.0826027491940167f,    -0.0826027491940166f,   -0.0826027491940166f,
      0.0826027491940166f};
  const int kAmbisonicOrder = 1;
  const int kNumChannels = 4;
  const int kNumVirtualSpeakers = 16;
  std::vector<SphericalAngle> sphere16_angles{
      SphericalAngle::FromDegrees(0.0f, -13.6f),
      SphericalAngle::FromDegrees(45.0f, 13.6f),
      SphericalAngle::FromDegrees(90.0f, -13.6f),
      SphericalAngle::FromDegrees(135.0f, 13.6f),
      SphericalAngle::FromDegrees(180.0f, -13.6f),
      SphericalAngle::FromDegrees(-135.0f, 13.6f),
      SphericalAngle::FromDegrees(-90.0f, -13.6f),
      SphericalAngle::FromDegrees(-45.0f, 13.6f),
      SphericalAngle::FromDegrees(0.0f, 51.5f),
      SphericalAngle::FromDegrees(90.0f, 51.5f),
      SphericalAngle::FromDegrees(180.0f, 51.5f),
      SphericalAngle::FromDegrees(-90.0f, 51.5f),
      SphericalAngle::FromDegrees(45.0f, -51.5f),
      SphericalAngle::FromDegrees(135.0f, -51.5f),
      SphericalAngle::FromDegrees(-135.0f, -51.5f),
      SphericalAngle::FromDegrees(-45.0f, -51.5f)};

  std::unique_ptr<AmbisonicCodecImpl<>> codec_first_order_sphere16(
      new AmbisonicCodecImpl<>(kAmbisonicOrder, sphere16_angles));
  const AmbisonicCodecImpl<>::DecoderMatrix decoder_matrix =
      codec_first_order_sphere16->GetDecoderMatrix();
  // Check if the size of the decoding matrix is correct.
  ASSERT_EQ(decoder_matrix.size(), kNumVirtualSpeakers * kNumChannels);
  // Check each coefficient against MATLAB.
  for (size_t i = 0; i < kExpectedDecoderCoefficients.size(); ++i) {
    EXPECT_NEAR(kExpectedDecoderCoefficients[i], decoder_matrix(i),
                kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
