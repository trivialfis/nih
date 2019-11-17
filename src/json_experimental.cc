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
#include "nih/json_experimental.hh"
#include "nih/json_recursive_reader.hh"
#include "nih/json_writer_experimental.hh"

namespace nih {
namespace experimental {

size_t constexpr ValueImpl::kElementEnd;

void Value::Accept(JsonWriter &writer) {
  // Here this object is assumed to be already initialized.
  switch (this->get_type()) {
  case ValueKind::kFalse: {
    writer.HandleFalse();
    break;
  }
  case ValueKind::kTrue: {
    writer.HandleTrue();
    break;
  }
  case ValueKind::kNull: {
    writer.HandleNull();
    break;
  }
  case ValueKind::kInteger: {
    writer.HandleInteger(this->GetInt());
    break;
  }
  case ValueKind::kNumber: {
    writer.HandleFloat(this->GetFloat());
    break;
  }
  case ValueKind::kString: {
    auto str = this->GetString();
    writer.HandleString(str);
    break;
  }
  case ValueKind::kArray: {
    writer.BeginArray();
    for (size_t it = 0; it < this->array_table_.size(); ++it) {
      auto tree = json_tree_.Access();
      if (array_table_[it] != kElementEnd) {
        auto value = this->GetArrayElem(it);
        value.Accept(writer);
      }
      if (it != this->array_table_.size() - 1) {
        writer.Comma();
      }
    }
    writer.EndArray();
    break;
  }
  case ValueKind::kObject: {
    writer.BeginObject();
    auto tree = json_tree_.Access();
    for (size_t i = 0; i < this->object_table_.size(); ++i) {
      ObjectElement const& elem = object_table_[i];
      ConstStringRef key((char *)&(data_stack_.Access()[elem.key_begin]),
                         elem.key_end - elem.key_begin);
      writer.HandleString(key);

      writer.KeyValue();

      ValueKind kind = JsonTypeHandler::GetType(tree[elem.value]);
      ValueImplT value{kind, elem.value, json_tree_.Data(),
                       data_stack_.Data()};
      value.Accept(writer);
      if (i != this->object_table_.size() - 1) {
        writer.Comma();
      }
    }
    writer.EndObject();
    break;
  }
  }
}

Document Document::Load(StringRef json_str) {
  Document doc(false);
  doc._tree_storage.reserve(json_str.size() * 2);
  {
    // FIXME(trivialfis): Don't copy
    doc._data_storage.reserve(json_str.size() * 2);
  }

  JsonRecursiveReader reader(json_str, &(doc.value));

  std::tie(doc.err_code_, doc.last_character) = reader.Parse();
  NIH_ASSERT(doc.value.IsObject()) << KindStr(doc.value.get_type());

  return std::move(doc);
}

std::string Document::Dump() {
  NIH_ASSERT(err_code_ == jError::kSuccess) << ErrStr();
  if (!value.finalised_) {
    value.EndObject();
  }
  NIH_ASSERT(value.finalised_);
  std::string result;
  JsonWriter writer;

  this->value.Accept(writer);
  writer.TakeResult(&result);

  return result;
}

}  // namespace experimental
}  // namespace nih