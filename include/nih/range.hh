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
#ifndef _RANGE_HH_
#define _RANGE_HH_

#include <iterator>
#include "nih/primitives.hh"

namespace nih {

class Range {
  class RangeIterator {
    NihInt index_;
    NihInt step_;

   public:
    using iterator_category = std::bidirectional_iterator_tag;

   public:
    RangeIterator() = default;
    RangeIterator(NihInt ind, NihInt step) :
        index_{ind}, step_{step} {}
    RangeIterator(RangeIterator&& other) = default;
    RangeIterator(RangeIterator const& other) = default;

    RangeIterator& operator=(RangeIterator&& other) {
      index_ = other.index_;
      step_ = other.step_;
      return *this;
    }
    RangeIterator& operator=(RangeIterator const& other) {
      index_ = other.index_;
      step_ = other.step_;
      return *this;
    }

    NihInt operator*() const { return index_; }

    bool operator==(RangeIterator const& rhs) const {
      return index_ == rhs.index_; };
    bool operator!=(RangeIterator const& rhs) const {
      return !operator==(rhs); }

    RangeIterator& operator++() {
      index_ += step_;
      return *this;
    }
    RangeIterator operator++(int) {
      auto ret = *this;
      ++(*this);
      return ret;
    }
    RangeIterator& operator--() {
      index_ -= step_;
      return *this;
    }
    RangeIterator operator--(int) {
      auto ret = *this;
      ++(*this);
      return ret;
    }
  };

  RangeIterator start_;
  RangeIterator end_;

 public:
  using iterator = RangeIterator;        // NOLINT
  using const_iterator = const iterator; // NOLINT
  using difference_type = NihInt;        // NOLINT

  Range(NihInt start, NihInt end, NihInt step=1) :
      start_{start, step}, end_{end, step} {}
  Range(NihInt end):
      start_{0, 1}, end_{end, 1} {}
  ~Range() = default;

  iterator       begin()        { return cbegin(); }
  iterator       end()          { return cend();   }
  const_iterator cbegin() const { return start_;   }
  const_iterator cend()   const { return end_;     }
};

}  // namespace nih

#endif  // _RANGE_HH_