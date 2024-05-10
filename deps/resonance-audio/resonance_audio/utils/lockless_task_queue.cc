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

#include "utils/lockless_task_queue.h"

#include <limits>

#include "base/logging.h"

namespace vraudio {

namespace {

// Reserved index representing an invalid list index.
constexpr uint64_t kInvalidIndex = std::numeric_limits<uint32_t>::max();

// Maximum number of producers.
constexpr uint64_t kMaxProducers = kInvalidIndex - 1;

}  // namespace

LocklessTaskQueue::LocklessTaskQueue(size_t max_tasks) {
  CHECK_GT(max_tasks, 0U);
  CHECK_LE(max_tasks, kMaxProducers);
  Init(max_tasks);
}

LocklessTaskQueue::~LocklessTaskQueue() { Clear(); }

void LocklessTaskQueue::Post(Task&& task) {
  const TagAndIndex free_node_idx = PopNodeFromList(&free_list_head_idx_);
  if (GetIndex(free_node_idx) == kInvalidIndex) {
    LOG(WARNING) << "Queue capacity reached - dropping task";
    return;
  }
  nodes_[GetIndex(free_node_idx)].task = std::move(task);
  PushNodeToList(&task_list_head_idx_, free_node_idx);
}

void LocklessTaskQueue::Execute() {
  const TagAndIndex old_flag_with_invalid_index =
      (GetFlag(task_list_head_idx_) << 32) + kInvalidIndex;
  const TagAndIndex old_task_list_head_idx =
      task_list_head_idx_.exchange(old_flag_with_invalid_index);
  ProcessTaskList(old_task_list_head_idx, true /*execute_tasks*/);
}

void LocklessTaskQueue::Clear() {
  const TagAndIndex old_flag_with_invalid_index =
      (GetFlag(task_list_head_idx_) << 32) + kInvalidIndex;
  const TagAndIndex old_task_list_head_idx =
      task_list_head_idx_.exchange(old_flag_with_invalid_index);
  ProcessTaskList(old_task_list_head_idx, false /*execute_tasks*/);
}

LocklessTaskQueue::TagAndIndex LocklessTaskQueue::IncreaseTag(
    TagAndIndex tag_and_index) {
  // The most significant 32 bits a reserved for tagging. Overflows are
  // acceptable.
  return tag_and_index + (static_cast<uint64_t>(1) << 32);
}

LocklessTaskQueue::TagAndIndex LocklessTaskQueue::GetIndex(
    TagAndIndex tag_and_index) {
  // The least significant 32 bits a reserved for the index.
  return tag_and_index & std::numeric_limits<uint32_t>::max();
}

// Extracts the flag in the most significant 32 bits from a TagAndIndex;
LocklessTaskQueue::TagAndIndex LocklessTaskQueue::GetFlag(
    TagAndIndex tag_and_index) {
  // The most significant 32 bits a reserved for the flag.
  return tag_and_index >> 32;
}

void LocklessTaskQueue::PushNodeToList(
    std::atomic<TagAndIndex>* list_head_idx_ptr, TagAndIndex node_idx) {
  DCHECK(list_head_idx_ptr);
  TagAndIndex list_head_idx;
  do {
    list_head_idx = list_head_idx_ptr->load();
    nodes_[GetIndex(node_idx)].next = list_head_idx;
  } while (!std::atomic_compare_exchange_strong(list_head_idx_ptr,
                                                &list_head_idx, node_idx));
}

LocklessTaskQueue::TagAndIndex LocklessTaskQueue::PopNodeFromList(
    std::atomic<TagAndIndex>* list_head_idx_ptr) {
  DCHECK(list_head_idx_ptr);
  TagAndIndex list_head_idx;
  TagAndIndex list_head_next;
  do {
    list_head_idx = list_head_idx_ptr->load();
    if (GetIndex(list_head_idx) == kInvalidIndex) {
      // End of list reached.
      return kInvalidIndex;
    }
    list_head_next = nodes_[GetIndex(list_head_idx)].next;
  } while (!std::atomic_compare_exchange_strong(
      list_head_idx_ptr, &list_head_idx, list_head_next));
  return IncreaseTag(list_head_idx);
}

void LocklessTaskQueue::ProcessTaskList(TagAndIndex list_head_idx,
                                        bool execute) {
  TagAndIndex node_itr = list_head_idx;
  while (GetIndex(node_itr) != kInvalidIndex) {
    Node* node = &nodes_[GetIndex(node_itr)];
    TagAndIndex next_node = node->next;
    temp_tasks_.emplace_back(std::move(node->task));
    node->task = nullptr;
    PushNodeToList(&free_list_head_idx_, node_itr);
    node_itr = next_node;
  }

  if (execute) {
    // Execute tasks in reverse order.
    for (std::vector<Task>::reverse_iterator task_itr = temp_tasks_.rbegin();
         task_itr != temp_tasks_.rend(); ++task_itr) {
      if (*task_itr != nullptr) {
        (*task_itr)();
      }
    }
  }
  temp_tasks_.clear();
}

void LocklessTaskQueue::Init(size_t num_nodes) {
  nodes_.resize(num_nodes);
  temp_tasks_.reserve(num_nodes);

  // Initialize free list.
  free_list_head_idx_ = 0;
  for (size_t i = 0; i < num_nodes - 1; ++i) {
    nodes_[i].next = i + 1;
  }
  nodes_[num_nodes - 1].next = kInvalidIndex;

  // Initialize task list.
  task_list_head_idx_ = kInvalidIndex;
}

}  // namespace vraudio
