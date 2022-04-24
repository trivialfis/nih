/*
 * Copyright 2019-2022 The NIH Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied.  See the License for the specific language
 * governing permissions and limitations under the License.
 *
 */

#include "nih/memory.h"

#include "nih/String.h"

namespace nih {

MemInfo::MemInfo(bool realtime) : _realtime(realtime) {
  fin.open(kInfoPath);
  this->refresh();
}

size_t MemInfo::query(std::string name) {
  if (_realtime) {
    refresh();
  }
  return _meminfo.at(name);
}

void MemInfo::refresh() {
  std::string line;
  std::vector<std::string> results;
  while (std::getline(fin, line)) {
    split(line, &results, [](char c) { return c == ':'; });
  }

  size_t total = results.size();
  for (size_t i = 0; i < total - 1; i += 2) {
    auto const& name = results.at(i);
    auto const& str_value = results.at(i + 1);
    size_t value = str2n<size_t>(str_value);
    _meminfo[name] = value;
  }

  fin.clear();                  // clear fail and eof bits
  fin.seekg(0, std::ios::beg);  // back to the start!
}

size_t MemInfo::memTotal() { return query("MemTotal"); }

size_t MemInfo::memFree() { return query("MemFree"); }

size_t MemInfo::memAvailable() { return query("MemAvailable"); }

size_t MemInfo::buffers() { return query("Buffers"); }

size_t MemInfo::cached() { return query("Cached"); }

size_t MemInfo::swapCached() { return query("SwapCached"); }

size_t MemInfo::active() { return query("Active"); }

size_t MemInfo::inactive() { return query("Inactive"); }

MemInfo::~MemInfo() { fin.close(); }

}  // namespace nih