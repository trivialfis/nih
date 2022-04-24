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
#ifndef _RANGE_HH_
#define _RANGE_HH_

#include <iterator>

namespace nih {

class Range {
  class RangeIterator {
    std::int64_t _index;
    std::int64_t _step;

   public:
    using iterator_category = std::bidirectional_iterator_tag;  // NOLINT

   public:
    RangeIterator() = default;
    RangeIterator(std::int64_t ind, std::int64_t step) : _index{ind}, _step{step} {}
    RangeIterator(RangeIterator&& other) = default;
    RangeIterator(RangeIterator const& other) = default;

    RangeIterator& operator=(RangeIterator&& other) {
      std::swap(_index, other._index);
      std::swap(_step, other._step);
      return *this;
    }
    RangeIterator& operator=(RangeIterator const& other) = default;

    std::int64_t operator*() const { return _index; }

    bool operator==(RangeIterator const& rhs) const { return _index == rhs._index; };
    bool operator!=(RangeIterator const& rhs) const { return !operator==(rhs); }

    RangeIterator& operator++() {
      _index += _step;
      return *this;
    }
    RangeIterator operator++(int) {
      auto ret = *this;
      ++(*this);
      return ret;
    }
    RangeIterator& operator--() {
      _index -= _step;
      return *this;
    }
    RangeIterator operator--(int) {
      auto ret = *this;
      ++(*this);
      return ret;
    }
  };

  RangeIterator _start;
  RangeIterator _end;

 public:
  using iterator = RangeIterator;         // NOLINT
  using const_iterator = const iterator;  // NOLINT
  using difference_type = std::int64_t;   // NOLINT

  Range(std::int64_t start, std::int64_t end, std::int64_t step = 1)
      : _start{start, step}, _end{end, step} {}
  explicit Range(std::int64_t end) : _start{0, 1}, _end{end, 1} {}
  ~Range() = default;

  iterator begin() { return cbegin(); }
  iterator end() { return cend(); }
  [[nodiscard]] const_iterator cbegin() const { return _start; }
  [[nodiscard]] const_iterator cend() const { return _end; }
};

}  // namespace nih

#endif  // _RANGE_HH_