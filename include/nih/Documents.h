/*
 * Copyright 2021 The Tungsten Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */
#ifndef TUNGSTEN_SUPPORT_DOCUMENTS_H_
#define TUNGSTEN_SUPPORT_DOCUMENTS_H_
#include <string>
#include <map>
#include "Logging.h"

namespace tungsten {
class DocManager {
  std::map<std::string, std::string> _store;

 public:
  template <typename T>
  DocManager& Register(std::string const& name) {
    _store[name] = std::string{T::doc()};
    return *this;
  }
  void Query(std::string const& name, std::string* content) {
    if (_store.find(name) == _store.cend()) {
      LOG(FATAL) << "Document for " << name << " not found.";
    }
    *content = _store.at(name);
  }

  static DocManager& getManager() {
    static DocManager manager;
    return manager;
  }
};

#define TS_REGISTER_MODEL_DOC(model)                                                               \
  auto static __##model##_doc = DocManager::getManager().Register<model>(#model);
}  // namespace tungsten
#endif  // TUNGSTEN_SUPPORT_DOCUMENTS_H_
