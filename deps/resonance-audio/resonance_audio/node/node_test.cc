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

#include "node/node.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace vraudio {

namespace {

class SourceNode : public Node {
 public:
  explicit SourceNode(bool output_nullptr)
      : output_nullptr_(output_nullptr), output_(this), next_value_(0) {}

  void Process() final {
    if (output_nullptr_) {
      // Output nullptr.
      output_.Write(nullptr);
    } else {
      output_.Write(&next_value_);
      next_value_++;
    }
  }

  bool CleanUp() final { return false; }

  const bool output_nullptr_;
  Node::Output<int*> output_;

 private:
  int next_value_;
};

class PassThrough : public Node {
 public:
  PassThrough() : output_(this) {}

  void Process() final { output_.Write(input_.Read()[0]); }

  bool CleanUp() final { return false; }

  Node::Input<int*> input_;
  Node::Output<int*> output_;
};

class IncNode : public Node {
 public:
  IncNode() : output_(this), inc_value_(0) {}

  void Process() final {
    inc_value_ = *input_.Read()[0];
    ++inc_value_;
    output_.Write(&inc_value_);
  }

  bool CleanUp() final { return false; }

  Node::Input<int*> input_;
  Node::Output<int*> output_;

 private:
  int inc_value_;
};

class SinkNode : public Node {
 public:
  void Process() final {}
  bool CleanUp() final { return false; }

  Node::Input<int*> input_;
};

// Class for testing multiple subscriber streams.
class NodeTest : public ::testing::Test {
 public:
  void SetUp() override {}
};

TEST_F(NodeTest, SourceSink) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto sink_node = std::make_shared<SinkNode>();
  sink_node->input_.Connect(source_node, &source_node->output_);

  auto& data0 = sink_node->input_.Read();
  EXPECT_EQ(data0.size(), 1U);
  EXPECT_EQ(*data0[0], 1);

  auto& data1 = sink_node->input_.Read();
  EXPECT_EQ(data1.size(), 1U);
  EXPECT_EQ(*data1[0], 2);
}

TEST_F(NodeTest, SourcePassThroughSink) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto copy_node = std::make_shared<PassThrough>();
  auto sink_node = std::make_shared<SinkNode>();
  sink_node->input_.Connect(copy_node, &copy_node->output_);
  copy_node->input_.Connect(source_node, &source_node->output_);

  auto& data0 = sink_node->input_.Read();
  EXPECT_EQ(data0.size(), 1U);
  EXPECT_EQ(*data0[0], 1);

  auto& data1 = sink_node->input_.Read();
  EXPECT_EQ(data1.size(), 1U);
  EXPECT_EQ(*data1[0], 2);
}

TEST_F(NodeTest, TwoSources) {
  static const bool kOutputNullptr = false;
  auto source_node_a = std::make_shared<SourceNode>(kOutputNullptr);
  auto source_node_b = std::make_shared<SourceNode>(kOutputNullptr);
  auto sink_node = std::make_shared<SinkNode>();

  sink_node->input_.Connect(source_node_a, &source_node_a->output_);
  sink_node->input_.Connect(source_node_b, &source_node_b->output_);
  EXPECT_EQ(source_node_a.use_count(), 2);
  EXPECT_EQ(source_node_b.use_count(), 2);
  EXPECT_EQ(sink_node.use_count(), 1);

  auto& data0 = sink_node->input_.Read();
  EXPECT_EQ(data0.size(), 2U);
  EXPECT_EQ(*data0[0], 1);
  EXPECT_EQ(*data0[1], 1);

  auto& data1 = sink_node->input_.Read();
  EXPECT_EQ(data1.size(), 2U);
  EXPECT_EQ(*data1[0], 2);
  EXPECT_EQ(*data1[1], 2);

  sink_node.reset();
  EXPECT_EQ(source_node_a.use_count(), 1);
  EXPECT_EQ(source_node_b.use_count(), 1);
}

TEST_F(NodeTest, DoubleSink) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto sink_node_a = std::make_shared<SinkNode>();
  auto sink_node_b = std::make_shared<SinkNode>();

  sink_node_a->input_.Connect(source_node, &source_node->output_);
  sink_node_b->input_.Connect(source_node, &source_node->output_);
  EXPECT_EQ(sink_node_a.use_count(), 1);
  EXPECT_EQ(sink_node_b.use_count(), 1);
  EXPECT_EQ(source_node.use_count(), 3);

  auto& dataA0 = sink_node_a->input_.Read();
  auto& dataB0 = sink_node_b->input_.Read();
  EXPECT_EQ(dataA0.size(), 1U);
  EXPECT_EQ(dataB0.size(), 1U);
  EXPECT_EQ(*dataA0[0], 1);
  EXPECT_EQ(*dataB0[0], 1);

  auto& dataA1 = sink_node_a->input_.Read();
  auto& dataB1 = sink_node_b->input_.Read();
  EXPECT_EQ(dataA1.size(), 1U);
  EXPECT_EQ(dataB1.size(), 1U);
  EXPECT_EQ(*dataA1[0], 2);
  EXPECT_EQ(*dataB1[0], 2);

  sink_node_a.reset();
  EXPECT_EQ(source_node.use_count(), 2);
  sink_node_b.reset();
  EXPECT_EQ(source_node.use_count(), 1);
}

TEST_F(NodeTest, DoubleSinkWithIncrement) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto inc_node = std::make_shared<IncNode>();
  auto sink_node_a = std::make_shared<SinkNode>();
  auto sink_node_b = std::make_shared<SinkNode>();

  sink_node_a->input_.Connect(source_node, &source_node->output_);
  sink_node_b->input_.Connect(inc_node, &inc_node->output_);
  inc_node->input_.Connect(source_node, &source_node->output_);

  auto& dataA0 = sink_node_a->input_.Read();
  auto& dataB0 = sink_node_b->input_.Read();
  EXPECT_EQ(dataA0.size(), 1U);
  EXPECT_EQ(dataB0.size(), 1U);
  EXPECT_EQ(*dataA0[0], 1);
  EXPECT_EQ(*dataB0[0], 2);

  auto& dataA1 = sink_node_a->input_.Read();
  auto& dataB1 = sink_node_b->input_.Read();
  EXPECT_EQ(dataA1.size(), 1U);
  EXPECT_EQ(dataB1.size(), 1U);
  EXPECT_EQ(*dataA1[0], 2);
  EXPECT_EQ(*dataB1[0], 3);
}

TEST_F(NodeTest, DisconnectSingleLink) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto sink_node = std::make_shared<SinkNode>();

  sink_node->input_.Connect(source_node, &source_node->output_);
  EXPECT_EQ(sink_node.use_count(), 1);
  EXPECT_EQ(source_node.use_count(), 2);

  auto& data0 = sink_node->input_.Read();
  EXPECT_EQ(data0.size(), 1U);
  EXPECT_EQ(*data0[0], 1);

  sink_node->input_.Disconnect(&source_node->output_);
  EXPECT_EQ(sink_node.use_count(), 1);
  EXPECT_EQ(source_node.use_count(), 1);

  auto& data1 = sink_node->input_.Read();
  EXPECT_EQ(data1.size(), 0U);
}

TEST_F(NodeTest, DisconnectIntermediate) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto inc_node = std::make_shared<IncNode>();
  auto sink_node = std::make_shared<SinkNode>();

  sink_node->input_.Connect(inc_node, &inc_node->output_);
  inc_node->input_.Connect(source_node, &source_node->output_);

  inc_node->input_.Disconnect(&source_node->output_);
  inc_node.reset();
  EXPECT_EQ(sink_node.use_count(), 1);
  EXPECT_EQ(source_node.use_count(), 1);
}

TEST_F(NodeTest, DisconnectMultiLink) {
  static const bool kOutputNullptr = false;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto inc_node = std::make_shared<IncNode>();
  auto sink_node_a = std::make_shared<SinkNode>();
  auto sink_node_b = std::make_shared<SinkNode>();
  auto sink_node_c = std::make_shared<SinkNode>();

  sink_node_a->input_.Connect(source_node, &source_node->output_);
  sink_node_b->input_.Connect(inc_node, &inc_node->output_);
  sink_node_c->input_.Connect(inc_node, &inc_node->output_);
  inc_node->input_.Connect(source_node, &source_node->output_);

  sink_node_a->input_.Disconnect(&source_node->output_);
  EXPECT_EQ(sink_node_a.use_count(), 1);
  EXPECT_EQ(sink_node_b.use_count(), 1);
  EXPECT_EQ(sink_node_c.use_count(), 1);
  EXPECT_EQ(inc_node.use_count(), 3);
  EXPECT_EQ(source_node.use_count(), 2);

  auto& dataA0 = sink_node_a->input_.Read();
  auto& dataB0 = sink_node_b->input_.Read();
  auto& dataC0 = sink_node_c->input_.Read();
  EXPECT_EQ(dataA0.size(), 0U);
  EXPECT_EQ(dataB0.size(), 1U);
  EXPECT_EQ(*dataB0[0], 2);
  EXPECT_EQ(dataC0.size(), 1U);
  EXPECT_EQ(*dataC0[0], 2);

  sink_node_b->input_.Disconnect(&inc_node->output_);
  EXPECT_EQ(sink_node_a.use_count(), 1);
  EXPECT_EQ(sink_node_b.use_count(), 1);
  EXPECT_EQ(sink_node_c.use_count(), 1);
  EXPECT_EQ(inc_node.use_count(), 2);
  EXPECT_EQ(source_node.use_count(), 2);

  auto& dataA1 = sink_node_a->input_.Read();
  auto& dataB1 = sink_node_b->input_.Read();
  auto& dataC1 = sink_node_c->input_.Read();
  EXPECT_EQ(dataA1.size(), 0U);
  EXPECT_EQ(dataB1.size(), 0U);
  EXPECT_EQ(dataC1.size(), 1U);
  EXPECT_EQ(*dataC1[0], 3);

  sink_node_c->input_.Disconnect(&inc_node->output_);
  EXPECT_EQ(sink_node_a.use_count(), 1);
  EXPECT_EQ(sink_node_b.use_count(), 1);
  EXPECT_EQ(sink_node_c.use_count(), 1);
  EXPECT_EQ(inc_node.use_count(), 1);
  EXPECT_EQ(source_node.use_count(), 2);

  auto& dataA2 = sink_node_a->input_.Read();
  auto& dataB2 = sink_node_b->input_.Read();
  auto& dataC2 = sink_node_c->input_.Read();
  EXPECT_EQ(dataA2.size(), 0U);
  EXPECT_EQ(dataB2.size(), 0U);
  EXPECT_EQ(dataC2.size(), 0U);

  inc_node->input_.Disconnect(&source_node->output_);
  EXPECT_EQ(sink_node_a.use_count(), 1);
  EXPECT_EQ(sink_node_b.use_count(), 1);
  EXPECT_EQ(inc_node.use_count(), 1);
  EXPECT_EQ(source_node.use_count(), 1);
}

TEST_F(NodeTest, NullPtrSourceMultipleClients) {
  static const bool kOutputNullptr = true;
  auto source_node = std::make_shared<SourceNode>(kOutputNullptr);
  auto sink_node_a = std::make_shared<SinkNode>();
  auto sink_node_b = std::make_shared<SinkNode>();

  sink_node_a->input_.Connect(source_node, &source_node->output_);
  sink_node_b->input_.Connect(source_node, &source_node->output_);

  auto& dataA = sink_node_a->input_.Read();
  auto& dataB = sink_node_b->input_.Read();

  EXPECT_TRUE(dataA.empty());
  EXPECT_TRUE(dataB.empty());
}

}  // namespace

}  // namespace vraudio
