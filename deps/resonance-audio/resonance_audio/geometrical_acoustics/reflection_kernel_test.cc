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

#include "geometrical_acoustics/reflection_kernel.h"

#include <cmath>
#include <functional>
#include <random>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "Eigen/Dense"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {

using Eigen::Quaternionf;
using Eigen::Vector3f;

class ReflectionKernelTest : public testing::Test {
 public:
  ReflectionKernelTest()
      : incident_ray_(Vector3f(0.0f, 0.0f, 0.0f).data(),
                      Vector3f(1.0f, 1.0f, 1.0f).normalized().data(), 0.0f,
                      AcousticRay::kInfinity, kEnergies,
                      AcousticRay::RayType::kSpecular, 0.0f),
        random_engine_(0),
        distribution_(0.0f, 1.0f) {}

  void SetUp() override {
    // Reseed the random number generator for every test to be deterministic
    // and remove flakiness in tests.
    random_engine_.seed(0);
    random_number_generator_ = [this] { return distribution_(random_engine_); };
  }

 protected:
  const std::array<float, kNumReverbOctaveBands> kEnergies = {
      {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f}};
  const std::array<float, kNumReverbOctaveBands> kUnitAbsorptionCoefficients = {
      {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
  const std::array<float, kNumReverbOctaveBands> kZeroAbsorptionCoefficients = {
      {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

  AcousticRay incident_ray_;
  std::default_random_engine random_engine_;
  std::uniform_real_distribution<float> distribution_;
  std::function<float()> random_number_generator_;
};

TEST_F(ReflectionKernelTest, EnergyAbsorptionTest) {
  const std::array<float, kNumReverbOctaveBands>& original_energies =
      incident_ray_.energies();
  {
    // An absorption coefficient of 1 means that no energy is reflected.
    const ReflectionKernel reflection(kUnitAbsorptionCoefficients, 0.0f,
                                      random_number_generator_);
    const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      EXPECT_FLOAT_EQ(reflected_ray.energies().at(i), 0.0f);
    }
  }
  {
    // An absorption coefficient of 0 means that all energy is reflected.
    const ReflectionKernel reflection(kZeroAbsorptionCoefficients, 0.0f,
                                      random_number_generator_);
    const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      EXPECT_FLOAT_EQ(reflected_ray.energies().at(i), original_energies.at(i));
    }
  }
  {
    // An absorption coefficient of x means that only (1.0 - x) of the energy
    // would be reflected.
    std::array<float, kNumReverbOctaveBands> absorption_coefficients = {
        {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f}};
    const ReflectionKernel reflection(absorption_coefficients, 0.0f,
                                      random_number_generator_);
    const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      EXPECT_FLOAT_EQ(
          reflected_ray.energies().at(i),
          (1.0f - absorption_coefficients.at(i)) * original_energies.at(i));
    }
  }
}

TEST_F(ReflectionKernelTest, ScatteringCoefficientTest) {
  // Setting the scattering coefficient to 0.3 means 30% of the rays are
  // reflected diffusely while 70% are reflected specularly.
  const ReflectionKernel reflection(kUnitAbsorptionCoefficients, 0.3f,
                                    random_number_generator_);

  float specular_fraction = 0.0f;
  float diffuse_fraction = 0.0f;
  const int num_samples = 10000;
  const float fraction_per_sample = 1.0f / static_cast<float>(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);
    (reflected_ray.type() == AcousticRay::RayType::kDiffuse
         ? diffuse_fraction
         : specular_fraction) += fraction_per_sample;
  }
  EXPECT_NEAR(diffuse_fraction, 0.3f, 0.01f);
  EXPECT_NEAR(specular_fraction, 0.7f, 0.01f);
}

TEST_F(ReflectionKernelTest, MultipleReflectionsTest) {
  const ReflectionKernel reflection(kUnitAbsorptionCoefficients, 0.0f,
                                    random_number_generator_);

  // Reflect |num_reflections| times. Validate that each reflected ray starts
  // where its incident ray ends. This is to simulate the effect of a typical
  // path tracer.
  const size_t num_reflections = 5;
  const float t_between_intersections = 2.0f;
  const float distance_between_intersections = t_between_intersections *
      Vector3f(incident_ray_.direction()).norm();

  AcousticRay ray = incident_ray_;
  float expected_prior_distance = 0.0f;
  for (size_t i = 0; i < num_reflections; ++i) {
    // Emulate an intersection at t = 2, with a plane whose (unnormalized)
    // normal being (0, 0, -1) for even iterations and (0, 0, 1) for odd
    // iterations.
    ray.set_t_far(t_between_intersections);
    const float even_normal[3] = {0.0f, 0.0f, -1.0f};
    const float odd_normal[3] = {0.0f, 0.0f, 1.0f};
    ray.set_intersected_geometry_normal((i % 2 == 0) ? even_normal
                                                     : odd_normal);
    const AcousticRay reflected_ray = reflection.Reflect(ray);

    // Check that the reflected ray starts where the original ray ends.
    const Vector3f& expected_reflected_ray_origin =
        Vector3f(ray.origin()) + ray.t_far() * Vector3f(ray.direction());
    ExpectFloat3Close(reflected_ray.origin(),
                      expected_reflected_ray_origin.data());
    EXPECT_FLOAT_EQ(reflected_ray.t_near(), AcousticRay::kRayEpsilon);

    // Check that the |prior_distance| field accumulates the distance traveled.
    expected_prior_distance += distance_between_intersections;
    EXPECT_FLOAT_EQ(reflected_ray.prior_distance(), expected_prior_distance);

    // Continue tracing the reflected ray.
    ray = reflected_ray;
  }
}

TEST_F(ReflectionKernelTest, SpecularReflectionTest) {
  // Emulate an intersection at t = 2, with a plane whose (unnormalized) normal
  // is (-1, -1, 0).
  incident_ray_.set_t_far(2.0f);
  const float normal[3] = {-1.0f, -1.0f, 0.0f};
  incident_ray_.set_intersected_geometry_normal(normal);

  // Setting the scattering coefficient to zero means to always reflect
  // specularly.
  const ReflectionKernel reflection(kUnitAbsorptionCoefficients, 0.0f,
                                    random_number_generator_);
  const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);

  // Validate non-directional data.
  EXPECT_FLOAT_EQ(reflected_ray.t_near(), AcousticRay::kRayEpsilon);
  EXPECT_EQ(reflected_ray.type(), AcousticRay::RayType::kSpecular);
  const float expected_reflected_origin[3] = {1.15470053838f, 1.15470053838f,
                                              1.15470053838f};
  ExpectFloat3Close(reflected_ray.origin(), expected_reflected_origin);

  // Validate the reflected direction.
  const Vector3f& expected_reflected_direction =
      Vector3f(-1.0f, -1.0f, 1.0f).normalized();
  ExpectFloat3Close(reflected_ray.direction(),
                    expected_reflected_direction.data());
}

TEST_F(ReflectionKernelTest, DiffuseReflectionTest) {
  // Emulate an intersection at t = 2, with a plane whose (unnormalized) normal
  // is (-1, -1, 0).
  incident_ray_.set_t_far(2.0f);
  const float normal[3] = {-1.0f, -1.0f, 0.0f};
  incident_ray_.set_intersected_geometry_normal(normal);

  // Setting the scattering coefficient to 1.0 means to always reflect
  // diffusely.
  const ReflectionKernel reflection(kUnitAbsorptionCoefficients, 1.0f,
                                    random_number_generator_);
  const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);

  // Validate non-directional data.
  EXPECT_FLOAT_EQ(reflected_ray.t_near(), AcousticRay::kRayEpsilon);
  EXPECT_EQ(reflected_ray.type(), AcousticRay::RayType::kDiffuse);

  // Validate the reflected directions.
  // For a reflected ray whose direction distribution is cosine-weighted over a
  // hemisphere:
  // - The PDF of theta is 2 sin(theta) cos(theta), and the CDF is sin^2(theta).
  // - The PDF of phi is 1 / 2 pi, and the CDF is 0.5 + phi / 2 pi.
  const Vector3f& plane_unit_normal =
      Vector3f(incident_ray_.intersected_geometry_normal()).normalized();
  ValidateDistribution(100000, 100, [&reflection, &plane_unit_normal, this]() {
    const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);
    const Vector3f& local_direction =
        Quaternionf::FromTwoVectors(plane_unit_normal, Vector3f::UnitZ()) *
        Vector3f(reflected_ray.direction());
    const float cos_theta = local_direction.z();
    const float sin_theta_square = 1.0f - cos_theta * cos_theta;
    return sin_theta_square;
  });

  ValidateDistribution(100000, 100, [&reflection, &plane_unit_normal, this]() {
    const AcousticRay reflected_ray = reflection.Reflect(incident_ray_);
    const Vector3f& local_direction =
        Quaternionf::FromTwoVectors(plane_unit_normal, Vector3f::UnitZ()) *
        Vector3f(reflected_ray.direction());
    const float phi = std::atan2(local_direction.y(), local_direction.x());
    return 0.5f + phi / 2.0f / static_cast<float>(M_PI);
  });
}

TEST_F(ReflectionKernelTest, DiffuseRainReflectionTest) {
  // Emulate an intersection at t = 2, with a plane whose (unnormalized) normal
  // is (-1, -1, 0).
  incident_ray_.set_t_far(2.0f);
  const float normal[3] = {-1.0f, -1.0f, 0.0f};
  incident_ray_.set_intersected_geometry_normal(normal);

  // Setting the scattering coefficient to 1.0 means to always reflect
  // diffusely.
  const ReflectionKernel reflection(kUnitAbsorptionCoefficients, 1.0f,
                                    random_number_generator_);
  const AcousticRay reference_reflected_ray = reflection.Reflect(incident_ray_);

  // Generate a diffuse-rain ray for a listener on the same side of the plane.
  float direction_pdf = -1.0f;
  const Vector3f same_side_listener_position(0.0f, 0.0f, 2.0f);
  AcousticRay diffuse_rain_ray;
  reflection.ReflectDiffuseRain(incident_ray_, reference_reflected_ray,
                                same_side_listener_position, &direction_pdf,
                                &diffuse_rain_ray);

  // The direction of the diffuse-rain ray should be from the reflection point
  // (the origin of the |reference_reflected_ray|) to the listener position.
  const Vector3f reflection_point_to_listener_position =
      same_side_listener_position - Vector3f(reference_reflected_ray.origin());
  const Vector3f expected_direction =
      reflection_point_to_listener_position.normalized();
  ExpectFloat3Close(diffuse_rain_ray.direction(), expected_direction.data());

  // The output |direction_pdf| should be cos(theta) / pi, where theta is the
  // angle between the normal of the reflecting surface and the expected
  // direction.
  const float cos_theta = expected_direction.dot(Vector3f(normal).normalized());
  EXPECT_FLOAT_EQ(direction_pdf, cos_theta / kPi);

  // Verify that the |t_far| - |t_near| is the distance between the reflection
  // point and the listener position.
  EXPECT_FLOAT_EQ(diffuse_rain_ray.t_far() - diffuse_rain_ray.t_near(),
                  reflection_point_to_listener_position.norm());

  // Verify that other data are the same as |reference_reflected_ray|.
  ExpectFloat3Close(diffuse_rain_ray.origin(),
                    reference_reflected_ray.origin());
  EXPECT_FLOAT_EQ(diffuse_rain_ray.t_near(), reference_reflected_ray.t_near());
  ExpectFloat3Close(diffuse_rain_ray.intersected_geometry_normal(),
                    reference_reflected_ray.intersected_geometry_normal());
  for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
    EXPECT_FLOAT_EQ(diffuse_rain_ray.energies()[band],
                    reference_reflected_ray.energies()[band]);
  }
  EXPECT_EQ(diffuse_rain_ray.type(), reference_reflected_ray.type());
  EXPECT_FLOAT_EQ(diffuse_rain_ray.prior_distance(),
                  reference_reflected_ray.prior_distance());

  // Generate a diffuse-rain ray for a listener on the different side of the
  // plane and expect that the output |direction_pdf| is zero, meaning there is
  // no chance that the diffuse-rain ray is in that direction.
  const Vector3f different_side_listener_position(10.0f, 10.0f, 10.0f);
  reflection.ReflectDiffuseRain(incident_ray_, reference_reflected_ray,
                                different_side_listener_position,
                                &direction_pdf, &diffuse_rain_ray);
  EXPECT_FLOAT_EQ(direction_pdf, 0.0f);
}

}  // namespace

}  // namespace vraudio
