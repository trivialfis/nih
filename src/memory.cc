/* This file is part of NIH.
 *
 * Copyright (c) 2019 Jiaming Yuan <jm.yuan@outlook.com>
 *
 * NIH is free software: you can redistribute it and/or modify it under the
 * terms of the Lesser GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * NIH is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Lesser GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with NIH.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "nih/memory.hh"
#include "nih/strings.hh"

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
    split(line, &results, [](char c){ return c == ':'; });
  }

  size_t total = results.size();
  for (size_t i = 0; i < total - 1; i += 2) {
    auto const& name = results.at(i);
    auto const& str_value = results.at(i+1);
    size_t value = str2n<size_t>(str_value);
    _meminfo[name] = value;
  }

  fin.clear();                 // clear fail and eof bits
  fin.seekg(0, std::ios::beg); // back to the start!
}

size_t MemInfo::memTotal() {
  return query("MemTotal");
}

size_t MemInfo::memFree() {
  return query("MemFree");
}

size_t MemInfo::memAvailable() {
  return query("MemAvailable");
}

size_t MemInfo::buffers() {
  return query("Buffers");
}

size_t MemInfo::cached() {
  return query("Cached");
}

size_t MemInfo::swapCached() {
  return query("SwapCached");
}

size_t MemInfo::active() {
  return query("Active");
}

size_t MemInfo::inactive() {
  return query("Inactive");
}

MemInfo::~MemInfo() {
  fin.close();
}

}  // namespace nih