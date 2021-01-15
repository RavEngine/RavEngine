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

#ifndef RESONANCE_AUDIO_UTILS_LOCKLESS_TASK_QUEUE_H_
#define RESONANCE_AUDIO_UTILS_LOCKLESS_TASK_QUEUE_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

namespace vraudio {

// Lock-less task queue which is thread safe for concurrent task producers and
// single task consumers.
class LocklessTaskQueue {
 public:
  // Alias for the task closure type.
  typedef std::function<void()> Task;

  // Constructor. Preallocates nodes on the task queue list.
  //
  // @param max_tasks Maximum number of tasks on the task queue.
  explicit LocklessTaskQueue(size_t max_tasks);

  ~LocklessTaskQueue();

  // Posts a new task to task queue.
  //
  // @param task Task to process.
  void Post(Task&& task);

  // Executes all tasks on the task queue.
  void Execute();

  // Removes all tasks on the task queue.
  void Clear();

 private:
  // To prevent ABA problems during thread synchronization, the most significant
  // 32 bits of this index type are reserved for a continuously increasing
  // tag counter. This prevents cases where nodes on the head appears to be
  // untouched during the preparation of a push operation but instead they have
  // been popped and pushed back during a context switch.
  typedef uint64_t TagAndIndex;

  // Node to model a single-linked list.
  struct Node {
    Node() = default;

    // Dummy copy constructor to enable vector::resize allocation.
    Node(const Node& node) : next() {}

    // User task.
    LocklessTaskQueue::Task task;

    // Index to next node.
    std::atomic<TagAndIndex> next;
  };

  // Returned a TagAndIndex with increased tag.
  TagAndIndex IncreaseTag(TagAndIndex tag_and_index);

  // Extracts the index in the least significant 32 bits from a TagAndIndex.
  TagAndIndex GetIndex(TagAndIndex tag_and_index);

  // Extracts the flag in the most significant 32 bits from a TagAndIndex.
  TagAndIndex GetFlag(TagAndIndex tag_and_index);

  // Pushes a node to the front of a list.
  //
  // @param list_head Index to list head.
  // @param node Index of node to be pushed to the front of the list.
  void PushNodeToList(std::atomic<TagAndIndex>* list_head, TagAndIndex node);

  // Pops a node from the front of a list.
  //
  // @param list_head Index to list head.
  // @return Index of front node, kInvalidIndex if list is empty.
  TagAndIndex PopNodeFromList(std::atomic<TagAndIndex>* list_head);

  // Iterates over list and moves all tasks to |temp_tasks_| to be executed in
  // FIFO order. All processed nodes are pushed back to the free list.
  //
  // @param list_head Index of head node of list to be processed.
  // @param execute If true, tasks on task list are executed.
  void ProcessTaskList(TagAndIndex list_head, bool execute);

  // Initializes task queue structures and preallocates task queue nodes.
  //
  // @param num_nodes Number of nodes to be initialized on free list.
  void Init(size_t num_nodes);

  // Index to head node of free list.
  std::atomic<TagAndIndex> free_list_head_idx_;

  // Index to head node of task list.
  std::atomic<TagAndIndex> task_list_head_idx_;

  // Holds preallocated nodes.
  std::vector<Node> nodes_;

  // Temporary vector to hold |Task|s in order to execute them in reverse order
  // (FIFO, instead of LIFO).
  std::vector<Task> temp_tasks_;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_UTILS_LOCKLESS_TASK_QUEUE_H_
