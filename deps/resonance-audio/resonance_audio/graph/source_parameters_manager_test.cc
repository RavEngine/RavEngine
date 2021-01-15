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

#include "graph/source_parameters_manager.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace vraudio {

namespace {

// Tests that the manager registers/unregisters source parameters as expected
// for given arbitrary source ids.
TEST(SourceParametersManagerTest, RegisterUnregisterTest) {
  const SourceId kSourceIds[] = {0, 1, 5, 10};

  // Initialize a new |SourceParametersManager|.
  SourceParametersManager source_parameters_manager;
  for (const auto source_id : kSourceIds) {
    // Verify that no parameters are registered for given |source_id|.
    EXPECT_TRUE(source_parameters_manager.GetParameters(source_id) == nullptr);
    // Verify that the parameters are initialized after |Register|.
    source_parameters_manager.Register(source_id);
    EXPECT_FALSE(source_parameters_manager.GetParameters(source_id) == nullptr);
    // Verify that the parameters are destroyed after |Unregister|.
    source_parameters_manager.Unregister(source_id);
    EXPECT_TRUE(source_parameters_manager.GetParameters(source_id) == nullptr);
  }
}

// Tests that the manager correctly applies and returns parameter values of a
// source for a given arbitrary modifier.
TEST(SourceParametersManagerTest, ParametersAccessTest) {
  const SourceId kSourceId = 1;
  const float kSourceGain = 0.25f;

  // Initialize a new |SourceParametersManager| and register the source.
  SourceParametersManager source_parameters_manager;
  source_parameters_manager.Register(kSourceId);
  // Modify the gain parameter.
  auto mutable_parameters =
      source_parameters_manager.GetMutableParameters(kSourceId);
  EXPECT_TRUE(mutable_parameters != nullptr);
  mutable_parameters->gain = kSourceGain;
  // Access the parameters to verify the gain value was applied correctly.
  const auto parameters = source_parameters_manager.GetParameters(kSourceId);
  EXPECT_TRUE(parameters != nullptr);
  EXPECT_EQ(kSourceGain, parameters->gain);
}

// Tests that the manager correctly executes a given arbitrary call to process
// all parameters for all the sources contained within.
TEST(SourceParametersManagerTest, ProcessAllParametersTest) {
  const SourceId kSourceIds[] = {0, 1, 2, 3, 4, 5};
  const float kDistanceAttenuation = 0.75f;
  const auto kProcess = [kDistanceAttenuation](SourceParameters* parameters) {
    parameters->distance_attenuation = kDistanceAttenuation;
  };

  // Initialize a new |SourceParametersManager| and register all the sources.
  SourceParametersManager source_parameters_manager;
  for (const auto source_id : kSourceIds) {
    source_parameters_manager.Register(source_id);
  }
  // Process all parameters to apply the distance attenuation.
  source_parameters_manager.ProcessAllParameters(kProcess);
  // Verify that the distance attenuation value was applied correctly to all the
  // sources.
  for (const auto source_id : kSourceIds) {
    const auto parameters = source_parameters_manager.GetParameters(source_id);
    EXPECT_TRUE(parameters != nullptr);
    EXPECT_EQ(kDistanceAttenuation, parameters->distance_attenuation);
  }
}

}  // namespace

}  // namespace vraudio
