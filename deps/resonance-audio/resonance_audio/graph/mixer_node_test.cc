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

#include "graph/mixer_node.h"

#include <algorithm>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/logging.h"
#include "graph/system_settings.h"
#include "node/sink_node.h"
#include "node/source_node.h"

namespace vraudio {

namespace {

// Helper class to detect destruction.
class DeletionDetector {
 public:
  explicit DeletionDetector(bool* node_deletion_flag)
      : node_deletion_flag_(node_deletion_flag) {}
  ~DeletionDetector() {
    if (node_deletion_flag_ != nullptr) {
      *node_deletion_flag_ = true;
    }
  }

 private:
  bool* node_deletion_flag_;
};

// Wraps |SourceNode| to detect its deletion.
class MySourceNode : public SourceNode, DeletionDetector {
 public:
  explicit MySourceNode(bool* node_deletion_flag)
      : SourceNode(), DeletionDetector(node_deletion_flag) {}

 protected:
  const AudioBuffer* AudioProcess() final { return nullptr; }
};

// Wraps |MixerNode| to detect its deletion.
class MyAudioMixerNode : public MixerNode, DeletionDetector {
 public:
  explicit MyAudioMixerNode(bool* node_deletion_flag,
                            const SystemSettings& system_settings)
      : MixerNode(system_settings, kNumMonoChannels),
        DeletionDetector(node_deletion_flag) {}
};

// Wraps |SinkNode| to detect its deletion.
class MySinkNode : public SinkNode, DeletionDetector {
 public:
  explicit MySinkNode(bool* node_deletion_flag)
      : SinkNode(), DeletionDetector(node_deletion_flag) {}
};

// Tests that the |MixerNode| keeps connected at the moment all of its sources
// are removed.
TEST(AudioNodesTest, cleanUpOnEmptyInputTest) {
  SystemSettings system_settings_(kNumMonoChannels, 128 /* frames_per_buffer */,
                                  48000 /* sample_rate_hz */);

  bool source_node_deleted = false;
  bool mixer_node_deleted = false;
  bool sink_node_deleted = false;

  auto sink_node = std::make_shared<MySinkNode>(&sink_node_deleted);

  {
    // Create a source and mixer node and connect it to sink node.
    auto source_node = std::make_shared<MySourceNode>(&source_node_deleted);
    auto mixer_node = std::make_shared<MyAudioMixerNode>(&mixer_node_deleted,
                                                         system_settings_);

    // Connect nodes.
    sink_node->Connect(mixer_node);
    mixer_node->Connect(source_node);

    // End of stream is marked in source node. Do not expect any data anymore.
    source_node->MarkEndOfStream();
  }

  EXPECT_FALSE(source_node_deleted);
  EXPECT_FALSE(mixer_node_deleted);
  EXPECT_FALSE(sink_node_deleted);

  sink_node->CleanUp();

  EXPECT_TRUE(source_node_deleted);
  EXPECT_FALSE(mixer_node_deleted);
  EXPECT_FALSE(sink_node_deleted);
}

}  // namespace

}  // namespace vraudio
