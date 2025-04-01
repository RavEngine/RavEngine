//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) Guillaume Blanc                                              //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#include "ozz/animation/runtime/skeleton.h"

#include <cstring>

#include "ozz/base/io/archive.h"
#include "ozz/base/log.h"
#include "ozz/base/maths/math_ex.h"
#include "ozz/base/maths/soa_math_archive.h"
#include "ozz/base/maths/soa_transform.h"
#include "ozz/base/memory/allocator.h"

namespace ozz {
namespace animation {

Skeleton::Skeleton(Skeleton&& _other) { *this = std::move(_other); }

Skeleton& Skeleton::operator=(Skeleton&& _other) {
  std::swap(allocation_, _other.allocation_);
  std::swap(joint_rest_poses_, _other.joint_rest_poses_);
  std::swap(joint_parents_, _other.joint_parents_);
  std::swap(joint_names_, _other.joint_names_);

  return *this;
}

Skeleton::~Skeleton() { Deallocate(); }

char* Skeleton::Allocate(size_t _chars_size, size_t _num_joints) {
  // Distributes buffer memory while ensuring proper alignment (serves larger
  // alignment values first).
  static_assert(alignof(math::SoaTransform) >= alignof(char*) &&
                    alignof(char*) >= alignof(int16_t) &&
                    alignof(int16_t) >= alignof(char),
                "Must serve larger alignment values first)");

  assert(allocation_ == nullptr && "Already allocated");

  // Early out if no joint.
  if (_num_joints == 0) {
    return nullptr;
  }

  // Rest poses have SoA format
  const size_t num_soa_joints = (_num_joints + 3) / 4;
  const size_t joint_rest_poses_size =
      num_soa_joints * sizeof(math::SoaTransform);
  const size_t names_size = _num_joints * sizeof(char*);
  const size_t joint_parents_size = _num_joints * sizeof(int16_t);
  const size_t buffer_size =
      names_size + _chars_size + joint_parents_size + joint_rest_poses_size;

  // Allocates whole buffer.
  auto* allocator = memory::default_allocator();
  allocation_ = allocator->Allocate(buffer_size, alignof(math::SoaTransform));
  span<byte> buffer = {static_cast<byte*>(allocation_), buffer_size};

  // Serves larger alignment values first.
  // Rest pose first, biggest alignment.
  joint_rest_poses_ = fill_span<math::SoaTransform>(buffer, num_soa_joints);

  // Then names array, second biggest alignment.
  joint_names_ = fill_span<char*>(buffer, _num_joints);

  // Parents, third biggest alignment.
  joint_parents_ = fill_span<int16_t>(buffer, _num_joints);

  // Remaining buffer will be used to store joint names.
  assert(buffer.size_bytes() == _chars_size &&
         "Whole buffer should be consumed");
  return reinterpret_cast<char*>(buffer.data());
}

void Skeleton::Deallocate() {
  memory::default_allocator()->Deallocate(allocation_);
  allocation_ = nullptr;
}

void Skeleton::Save(ozz::io::OArchive& _archive) const {
  const int32_t num_joints = this->num_joints();

  // Early out if skeleton's empty.
  _archive << num_joints;
  if (!num_joints) {
    return;
  }

  // Stores names. They are all concatenated in the same buffer, starting at
  // joint_names_[0].
  size_t chars_count = 0;
  for (int i = 0; i < num_joints; ++i) {
    chars_count += (std::strlen(joint_names_[i]) + 1) * sizeof(char);
  }
  _archive << static_cast<int32_t>(chars_count);
  _archive << ozz::io::MakeArray(joint_names_[0], chars_count);
  _archive << ozz::io::MakeArray(joint_parents_);
  _archive << ozz::io::MakeArray(joint_rest_poses_);
}

void Skeleton::Load(ozz::io::IArchive& _archive, uint32_t _version) {
  // Deallocate skeleton in case it was already used before.
  Deallocate();

  if (_version != 2) {
    log::Err() << "Unsupported Skeleton version " << _version << "."
               << std::endl;
    return;
  }

  int32_t num_joints;
  _archive >> num_joints;

  // Early out if skeleton's empty.
  if (!num_joints) {
    return;
  }

  // Read names.
  int32_t chars_count;
  _archive >> chars_count;

  // Allocates all skeleton data members.
  char* cursor = Allocate(chars_count, num_joints);

  // Reads name's buffer, they are all contiguous in the same buffer.
  _archive >> ozz::io::MakeArray(cursor, chars_count);

  // Fixes up array of pointers.
  for (int i = 0; i < num_joints;
       joint_names_[i] = cursor, cursor += std::strlen(cursor) + 1, ++i) {
  }

  _archive >> ozz::io::MakeArray(joint_parents_);
  _archive >> ozz::io::MakeArray(joint_rest_poses_);
}
}  // namespace animation
}  // namespace ozz
