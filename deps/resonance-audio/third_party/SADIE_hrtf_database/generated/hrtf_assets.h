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

#ifndef THIRD_PARTY_SADIE_HRTF_DATABASE_GENERATED_HRTF_ASSETS_H_
#define THIRD_PARTY_SADIE_HRTF_DATABASE_GENERATED_HRTF_ASSETS_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sadie {

// Note this class is automatically generated. Do not modify.
class HrtfAssets {
 public:
  // Lookups and retrieves a file from an asset class.
  //
  // @param filename: Filename to be retrieved.
  // @return std::string with raw file data. In case of an error, a nullptr is
  //     returned. Caller must take over the ownership of the returned data.
  std::unique_ptr<std::string> GetFile(const std::string& filename) const;

 private:
  typedef std::unordered_map<std::string, std::vector<unsigned char>>
      AssetDataMap;
  static const AssetDataMap kAssetMap;
};

}  // namespace sadie

#endif  // THIRD_PARTY_SADIE_HRTF_DATABASE_GENERATED_HRTF_ASSETS_H_
