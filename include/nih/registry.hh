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
#ifndef _REGISTRY_HH_
#define _REGISTRY_HH_

#include <functional>
#include <string>
#include <map>

namespace nih {

template <typename FType>
struct RegistryEntry {
 private:
  std::string _description;
  std::function<FType> _creator;

 public:
  RegistryEntry& describe(std::string description) {
    _description = description;
    return *this;
  }
  RegistryEntry& setCreator(std::function<FType> func) {
    _creator = func;
    return *this;
  }
  std::function<FType> getCreator() const {
    return _creator;
  }
};

template<typename EntryType>
class Registry {
 public:
  static std::map<std::string, EntryType>* getRegistry() {
    static std::map<std::string, EntryType> registry;
    return &registry;
  }
};


}      // namespace kapok

#endif  // _REGISTRY_HH_