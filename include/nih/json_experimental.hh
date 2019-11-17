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
#ifndef _NIH_JSON_EXPERIMENTAL_HH_
#define _NIH_JSON_EXPERIMENTAL_HH_

#include <cinttypes>
#include <vector>
#include <nih/span.hh>

#include <nih/logging.hh>

namespace nih {
namespace experimental {

/*\brief Types of JSON value.
 *
 * Inspired by sajson, this type is intentionally packed into 3 bits.
 */
enum class ValueKind : std::uint8_t {
  kTrue = 0x0,
  kFalse = 0x1,
  kInteger = 0x2,
  kNumber = 0x3,
  kString = 0x4,

  kArray = 0x5,
  kObject = 0x6,

  kNull = 0x7
};

std::string KindStr(ValueKind kind) {
  switch (kind) {
    case ValueKind::kTrue:
      return "ture";
    case ValueKind::kFalse:
      return "false";
    case ValueKind::kInteger:
      return "integer";
    case ValueKind::kNumber:
      return "number";
    case ValueKind::kString:
      return "string";
    case ValueKind::kArray:
      return "array";
    case ValueKind::kObject:
      return "object";
    case ValueKind::kNull:
      return "null";
  };
  return "";
}

/*! \brief A mutable string_view. */
template <typename CharT>
class StringRefImpl {
 public:
  using pointer = CharT*;
  using iterator = pointer;
  using const_iterator = pointer const;

  using value_type = CharT;
  using traits_type = std::char_traits<CharT>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using reference = CharT&;
  using const_reference = CharT const&;

 private:
  CharT* chars_;
  size_t size_;

 public:
  StringRefImpl() : chars_{nullptr}, size_{0} {}
  StringRefImpl(std::string& str) : chars_{&str[0]}, size_{str.size()} {}
  StringRefImpl(std::string const& str) : chars_{str.data()}, size_{str.size()} {}
  StringRefImpl(CharT* chars, size_t size) : chars_{chars}, size_{size} {}
  StringRefImpl(CharT* chars) : chars_{chars}, size_{traits_type::length(chars)} {}

  const_iterator cbegin() const { return chars_; }  // NOLINT
  const_iterator cend()   const { return chars_ + size(); }  // NOLINT

  iterator begin() { return chars_; }         // NOLINT
  iterator end() { return chars_ + size(); }  // NOLINT

  pointer data() const { return chars_; }     // NOLINT

  size_t size() const { return size_; };      // NOLINT
  CharT operator[](size_t i) const { return chars_[i]; }

  bool operator==(StringRefImpl const &that) {
    return Span<CharT>{this, size_} == Span<CharT> {that.data(), that.size()};
  }
  bool operator!=(StringRefImpl const &that) {
    return !(that == *this);
  }
  bool operator<(StringRefImpl const &that) {
    return Span<CharT>{chars_, size_} < Span<CharT> {that.data(), that.size()};
  }
  bool operator>(StringRefImpl const &that) {
    return Span<CharT>{chars_, size_} > Span<CharT> {that.data(), that.size()};
  }
  bool operator<=(StringRefImpl const &that) {
    return Span<CharT>{chars_, size_} <= Span<CharT> {that.data(), that.size()};
  }
  bool operator>=(StringRefImpl const &that) {
    return Span<CharT>{chars_, size_} >= Span<CharT> {that.data(), that.size()};
  }
};

/*\brief An iterator for looping through Object members.  Inspired by rapidjson. */
template <typename ValueType, bool IsConst>
class ElemIterator {
  size_t index_;
  typename std::conditional<IsConst, ValueType const*, ValueType *>::type ptr_;

 public:
  using reference = typename std::conditional<IsConst, ValueType const, ValueType>::type&;
  using difference_type = std::ptrdiff_t;
  using value_type = typename std::remove_reference<
      typename std::remove_cv<ValueType>::type>::type;

  ElemIterator() = default;
  ElemIterator(ValueType *p, size_t i) : ptr_{p}, index_{i} {}
  template <typename std::enable_if<IsConst>::type* = nullptr>
  ElemIterator(ValueType const *p, size_t i) : ptr_{p}, index_{i} {}
  ElemIterator(ElemIterator const &that) {
    index_ = that.index_;
    ptr_ = that.ptr_;
  }
  // prefix
  ElemIterator& operator++() {
    index_ ++;
    return *this;
  }
  // postfix
  ElemIterator operator++(int){
    auto ret = *this;
    ++(*this);
    return ret;
  }

  value_type operator*() {
    NIH_ASSERT(ptr_);
    auto v = ptr_->GetElemByIndex(index_);
    return v;
  }
  value_type operator[](difference_type i) const {
    NIH_ASSERT(ptr_);
    NIH_ASSERT_LT(i + index_, ptr_->Length());
    ptr_->GetElemByIndex(i + index_);
  }

  bool operator==(ElemIterator const& that) const {
    return index_ == that.index_ && ptr_ == that.ptr_;
  }
  bool operator!=(ElemIterator const &that) const {
    return !(that == *this);
  }
};

using StringRef = StringRefImpl<std::string::value_type>;
using ConstStringRef = StringRefImpl<std::string::value_type const>;

/*! \brief Commom utilitis for handling compact JSON type.  The type implementation is
 *  inspired by sajson. */
struct JsonTypeHandler {
 public:
  // number of bits required to store the type information.
  static size_t constexpr kTypeBits   { 3   };
  // 00...0111, get the first 3 bits where type information is stored.
  static size_t constexpr kTypeMask   { 0x7 };

 public:
  // return type information from typed offset.
  static ValueKind GetType(size_t typed_offset) {
    auto uint_type = static_cast<uint8_t>(typed_offset & kTypeMask);
    return static_cast<ValueKind>(uint_type);
  }
  // return offset information from typed offset.
  static size_t GetOffset(size_t typed_offset) {
    return typed_offset >> kTypeBits;
  }
  // Split up the typed offset into type and offset, combination of above functions.
  static std::pair<ValueKind, size_t> GetTypeOffset(size_t typed_offset) {
    auto uint_type = static_cast<uint8_t>(typed_offset & kTypeMask);
    auto offset = typed_offset >> kTypeBits;
    return {static_cast<ValueKind>(uint_type), offset};
  }
  // Pack type information into a tree pointer.
  static size_t constexpr MakeTypedOffset(size_t ptr, ValueKind kind) {
    return ptr << kTypeBits | static_cast<size_t>(kind);
  }
};

template <typename T>
class StorageView {
  std::vector<T> *storage_ref_;

public:
  StorageView(std::vector<T> *storage_ref) : storage_ref_{storage_ref} {}
  size_t Top() const { return storage_ref_->size(); }
  Span<T> Access() const { return Span<T>{*storage_ref_}; }
  void Resize(size_t n) { this->storage_ref_->resize(n); }
  void Expand(size_t n) { this->Resize(storage_ref_->size() + n); }
  std::vector<T> *Data() const { return storage_ref_; }
};

class JsonWriter;
class Document;

class ValueImpl {
 protected:
  // Using inheritence is not idea as Document would go out of scope before this base
  // class, while memory buffer is held in Document;
  friend Document;
  using ValueImplT = ValueImpl;

  // raw storage to JSON tree.
  StorageView<size_t> json_tree_;
  // raw storage to actual data.
  StorageView<std::string::value_type> data_stack_;
  // Storing the type information for this node.
  ValueKind kind_ { ValueKind::kNull };

 public:
  // A guard for invalid pointer.
  static size_t constexpr kElementEnd { std::numeric_limits<size_t>::max() };

  // This is a node in the singly linked list
  struct ObjectElement {
    /*\brief Address of value in JSON tree. */
    size_t value;
    /*\brief Address of key in data storage. */
    size_t key_begin;
    /*\brief Address of end of key in data storage. */
    size_t key_end;

    /*\brief Offset inside this struct to the value pointer. */
    // size_t constexpr static kValueOffset {2};

    friend std::ostream &operator<<(std::ostream &os, ObjectElement elem) {
      os << "begin: " << elem.key_begin << ", " << "end:" << elem.key_end << std::endl;
      return os;
    }
  };

  using iterator = ElemIterator<ValueImplT, false>;
  using const_iterator = ElemIterator<ValueImplT, true>;

  struct StringStorage {
    size_t beg;
    size_t end;
  };

 protected:
  // A `this` pointer with type information written in first 3 bits.
  size_t _tree_begin {0};
  bool is_view_ {false};
  bool finalised_ {true};

  void InitializeType(ValueKind kind) {
    NIH_ASSERT_EQ(static_cast<uint8_t>(this->kind_),
                  static_cast<uint8_t>(ValueKind::kNull)) << "You can not reset a value.";
    this->kind_ = kind;
  }

  void CheckType(ValueKind type) const {
    NIH_ASSERT_EQ(static_cast<std::uint8_t>(type), static_cast<std::uint8_t>(this->kind_));
  }
  void CheckType(ValueKind a, ValueKind b) const {
    NIH_ASSERT(static_cast<std::uint8_t>(a) == static_cast<std::uint8_t>(this->kind_) ||
               static_cast<std::uint8_t>(b) == static_cast<std::uint8_t>(this->kind_));
  }

  bool NeedFinalise() const {
    return !is_view_ && !finalised_ && (this->IsArray() || this->IsObject());
  }

 protected:
  // explict working memory for non-trivial types.
  std::vector<ObjectElement> object_table_;
  std::vector<size_t> array_table_;
  StringStorage  string_storage;

 protected:
  ValueImpl(std::vector<size_t> *tree_storage_ref,
            std::vector<std::string::value_type> *data_storage_ref)
      : json_tree_{tree_storage_ref}, data_stack_{data_storage_ref} {
  }

  ValueImpl(std::vector<size_t> *tree_storage_ref,
            std::vector<std::string::value_type> *data_storage_ref,
            size_t tree_beg)
      : json_tree_ {StorageView<size_t>{tree_storage_ref}},
        data_stack_ {StorageView<std::string::value_type>{data_storage_ref}},
        _tree_begin {tree_beg},
        kind_{ValueKind::kNull} {}

  // ValueImpl knows how to construct itself from data kind and a pointer to its storage
  ValueImpl(ValueKind kind, size_t tree_pointer,
            StorageView<size_t> tree_stack,
            StorageView<std::string::value_type> data_stack)
      : kind_{kind}, _tree_begin{tree_pointer}, is_view_{true},
        json_tree_{tree_stack}, data_stack_{data_stack} {
    switch (kind_) {
    case ValueKind::kInteger: {
      this->kind_ = ValueKind::kInteger;
      break;
    }
    case ValueKind::kNumber: {
      this->kind_ = ValueKind::kNumber;
      break;
    }
    case ValueKind::kObject: {
      this->kind_ = ValueKind::kObject;
      auto tree = json_tree_.Access();
      size_t table_offset = JsonTypeHandler::GetOffset(tree[_tree_begin]);
      size_t length = tree[table_offset];
      object_table_.resize(length);
      auto tree_ptr = table_offset + 1;
      for (size_t i = 0; i < length; ++i) {
        object_table_[i].key_begin = tree[tree_ptr];
        object_table_[i].key_end = tree[tree_ptr + 1];
        object_table_[i].value = tree[tree_ptr + 2];
        tree_ptr += 3;
      }
      break;
    }
    case ValueKind::kArray: {
      this->kind_ = ValueKind::kArray;
      auto tree = json_tree_.Access();
      size_t table_offset = JsonTypeHandler::GetOffset(tree[_tree_begin]);
      size_t length = tree[table_offset];
      array_table_.resize(length);
      auto tree_ptr = table_offset + 1;
      std::memcpy(array_table_.data(), tree.data() + tree_ptr, length * sizeof(size_t));
      break;
    }
    case ValueKind::kNull: {
      this->kind_ = ValueKind::kNull;
      break;
    }
    case ValueKind::kString: {
      this->kind_ = ValueKind::kString;
      auto tree = json_tree_.Access();
      auto storage_ptr = JsonTypeHandler::GetOffset(tree[_tree_begin]);
      string_storage.beg = tree[storage_ptr];
      string_storage.end = tree[storage_ptr + 1];
      break;
    }
    case ValueKind::kTrue: {
      this->kind_ = ValueKind::kTrue;
      break;
    }
    case ValueKind::kFalse: {
      this->kind_ = ValueKind::kFalse;
      break;
    }
    default: {
      LOG(FATAL) << "Invalid value type: " << static_cast<uint8_t>(kind_);
      break;
    }
    }
  }

  template <typename T>
  void SetNumber(T value, ValueKind kind) {
    InitializeType(kind);
    auto tree = json_tree_.Access();

    auto const current_data_pointer = data_stack_.Top();
    auto const current_tree_pointer = json_tree_.Top();
    tree[this->_tree_begin] =
        JsonTypeHandler::MakeTypedOffset(current_data_pointer, kind);
    data_stack_.Expand(sizeof(value));
    auto data = data_stack_.Access();
    std::memcpy(data.data() + current_data_pointer, (void*)&value, sizeof(value));
  }

  template <typename T>
  T GetNumber(ValueKind kind) const {
    NIH_ASSERT(this->kind_ == kind) << KindStr(kind_) << " vs: " << KindStr(kind);
    auto tree = json_tree_.Access();
    auto data_ptr = JsonTypeHandler::GetOffset(tree[_tree_begin]);
    auto data = data_stack_.Access();
    T value { T() };
    std::memcpy(&value, data.data() + data_ptr, sizeof(value));
    return value;
  }

 public:
   ValueImpl(ValueImpl const &that)  // = delete
       : _tree_begin{that._tree_begin}, json_tree_{that.json_tree_},
         data_stack_{that.data_stack_}, object_table_{that.object_table_},
         array_table_{that.array_table_}, kind_{that.kind_},
         finalised_{that.finalised_}, is_view_{that.is_view_} {}
  ValueImpl(ValueImpl&& that)
      :
      _tree_begin{that._tree_begin}, json_tree_{that.json_tree_},
      data_stack_{that.data_stack_}, object_table_{std::move(that.object_table_)},
      array_table_{std::move(that.array_table_)}, kind_{that.kind_},
      finalised_{that.finalised_}, is_view_{that.is_view_} {
        that.finalised_ = true;
      }

   virtual ~ValueImpl() {
     if (!this->NeedFinalise()) { return; }
     switch (this->get_type()) {
     case ValueKind::kObject: {
       this->EndObject();
       break;
     }
     case ValueKind::kArray: {
       this->EndArray();
       break;
     }
     default: {
       break;
     }
     }
   }

   bool IsObject() const { return kind_ == ValueKind::kObject; }
   bool IsArray() const { return kind_ == ValueKind::kArray; }
   bool IsString() const { return kind_ == ValueKind::kString; }

   bool IsInteger() const { return kind_ == ValueKind::kInteger; }
   bool IsNumber() const { return kind_ == ValueKind::kNumber; }
   bool IsTrue() const { return kind_ == ValueKind::kTrue; }
   bool IsFalse() const { return kind_ == ValueKind::kFalse; }
   bool IsNull() const { return kind_ == ValueKind::kNull; }

   size_t Length() const {
     CheckType(ValueKind::kArray, ValueKind::kObject);
     if (this->kind_ == ValueKind::kArray) {
       return this->array_table_.size();
     } else {
       return this->object_table_.size();
     }
  }

  ValueImpl SetTrue() {
    InitializeType(ValueKind::kTrue);
    return *this;
  }

  ValueImpl SetFalse() {
    InitializeType(ValueKind::kFalse);
    return *this;
  }

  ValueImpl SetNull() {
    InitializeType(ValueKind::kNull);
    return *this;
  }

  ValueImpl SetString(ConstStringRef string) {
    InitializeType(ValueKind::kString);
    auto current_tree_pointer = json_tree_.Top();
    auto tree = json_tree_.Access();
    tree[this->_tree_begin] = JsonTypeHandler::MakeTypedOffset(
        current_tree_pointer, ValueKind::kString);

    this->json_tree_.Expand(2);
    tree = json_tree_.Access();

    auto current_data_pointer = this->data_stack_.Top();

    this->data_stack_.Expand(string.size());
    auto data = data_stack_.Access();
    std::memcpy(data.data() + current_data_pointer, string.data(), string.size());

    string_storage.beg = current_data_pointer;
    string_storage.end = current_data_pointer + string.size();

    tree[current_tree_pointer] = string_storage.beg;
    tree[current_tree_pointer + 1] = string_storage.end;
    return *this;
  }

  ConstStringRef GetString() const {
    NIH_ASSERT(this->IsString());
    auto tree = json_tree_.Access();
    auto data = data_stack_.Access();
    return {&data[string_storage.beg], string_storage.end - string_storage.beg};
  }

  void SetInteger(int64_t i) {
    SetNumber(i, ValueKind::kInteger);
  }

  void SetFloat(float f) {
    SetNumber(f, ValueKind::kNumber);
  }

  float GetFloat() const {
    return GetNumber<float>(ValueKind::kNumber);
  }

  int64_t GetInt() const {
    return GetNumber<int64_t>(ValueKind::kInteger);
  }

  /*\brief Set this value to an array with pre-defined length. */
  ValueImpl SetArray(size_t length) {
    InitializeType(ValueKind::kArray);
    auto tree = json_tree_.Access();
    tree[_tree_begin] = JsonTypeHandler::MakeTypedOffset(kElementEnd,
                                                         ValueKind::kArray);
    array_table_.resize(length, kElementEnd);
    finalised_ = false;
    return *this;
  }

  ValueImpl GetArrayElem(size_t index) {
    NIH_ASSERT(this->IsArray());
    NIH_ASSERT_LT(index, array_table_.size());
    auto tree = json_tree_.Access();
    ValueKind kind;
    if (array_table_[index] == kElementEnd) {
      auto value = ValueImpl {json_tree_.Data(), data_stack_.Data(), json_tree_.Top()};
      array_table_[index] = json_tree_.Top();
      json_tree_.Expand(1);
      return value;
    } else {
      kind = JsonTypeHandler::GetType(tree[array_table_[index]]);
      ValueImpl value(kind, array_table_[index], json_tree_.Data(), data_stack_.Data());
      return value;
    }
  }

  ValueImpl SetArray() {
    array_table_.reserve(128);
    this->SetArray(0);
    return *this;
  }
  ValueImpl CreateArrayElem() {
    NIH_ASSERT(this->IsArray());
    auto tree = json_tree_.Access();
    auto value = ValueImpl {json_tree_.Data(), data_stack_.Data(), json_tree_.Top()};
    array_table_.push_back(json_tree_.Top());
    json_tree_.Expand(1);
    return value;
  }

  ValueImpl SetObject() {
    // initialize the value here.
    InitializeType(ValueKind::kObject);
    auto tree = json_tree_.Access();
    tree[_tree_begin] =
        JsonTypeHandler::MakeTypedOffset(_tree_begin, ValueKind::kObject);
    finalised_ = false;
    return *this;
  }

  ValueImpl CreateMember(ConstStringRef key) {
    NIH_ASSERT(this->IsObject());
    size_t const current_tree_pointer = json_tree_.Top();
    // allocate space for object element.
    json_tree_.Expand(sizeof(ObjectElement) / sizeof(size_t) + 1);
    auto tree = json_tree_.Access();

    size_t const current_data_pointer = data_stack_.Top();
    // allocate space for key
    data_stack_.Expand(key.size() * sizeof(ConstStringRef::value_type));
    auto data = data_stack_.Access();
    std::copy(key.cbegin(), key.cend(), data.begin() + current_data_pointer);

    ObjectElement elem;
    elem.key_begin = current_data_pointer;
    elem.key_end = current_data_pointer + key.size();
    elem.value = json_tree_.Top() - 1;

    object_table_.push_back(elem);

    auto value = ValueImpl{json_tree_.Data(), data_stack_.Data(), elem.value};
    return value;
  }

  void EndObject() {
    NIH_ASSERT(!finalised_);
    NIH_ASSERT(this->IsObject()) << " Kind:" << KindStr(kind_);
    size_t current_tree_pointer = json_tree_.Top();
    auto table_begin = current_tree_pointer;

    json_tree_.Expand(object_table_.size() * 3 + 1);  // +1 length
    auto tree = json_tree_.Access();
    tree[current_tree_pointer] = object_table_.size();
    current_tree_pointer += 1;

    for (size_t i = 0; i < object_table_.size(); ++i) {
      tree[current_tree_pointer] = object_table_[i].key_begin;
      tree[current_tree_pointer + 1] = object_table_[i].key_end;
      tree[current_tree_pointer + 2] = object_table_[i].value;
      current_tree_pointer += 3;
    }

    tree[_tree_begin] = JsonTypeHandler::MakeTypedOffset(
        table_begin, ValueKind::kObject);
    finalised_ = true;
  }

  void EndArray() {
    NIH_ASSERT(!finalised_);
    NIH_ASSERT(this->IsArray());
    size_t current_tree_pointer = json_tree_.Top();
    auto table_begin = current_tree_pointer;

    json_tree_.Expand(array_table_.size() + 1);
    auto tree = json_tree_.Access();
    tree[current_tree_pointer] = array_table_.size();
    current_tree_pointer += 1;

    std::copy(array_table_.cbegin(), array_table_.cend(),
              tree.begin() + current_tree_pointer);

    tree[_tree_begin] = JsonTypeHandler::MakeTypedOffset(
        table_begin, ValueKind::kArray);
    finalised_ = true;
  }

  ValueImplT GetElemByIndex(size_t index) const {
    NIH_ASSERT(this->IsObject());
    NIH_ASSERT_LT(index, object_table_.size());
    auto tree = json_tree_.Access();
    auto elem = object_table_[index];
    ValueKind kind;
    size_t offset_in_tree;
    std::tie(kind, offset_in_tree) =
        JsonTypeHandler::GetTypeOffset(tree[elem.value]);
    ValueImpl value{kind, elem.value, json_tree_.Data(), data_stack_.Data()};
    return value;
  }

  const_iterator FindMemberByKey(ConstStringRef key) const {
    NIH_ASSERT(this->IsObject());
    auto tree = json_tree_.Access();
    auto data = data_stack_.Access();
    for (size_t i = 0; i < object_table_.size(); ++i) {
      auto elem = object_table_[i];
      char const* c_str = &data[elem.key_begin];
      auto size = elem.key_end - elem.key_begin;
      auto str = ConstStringRef(c_str, size);
      bool equal = false;
      if (str.size() == key.size()) {
        equal = std::memcmp(c_str, key.data(), str.size()) == 0;
      } else {
        equal = false;
      }

      if (equal) {
        return const_iterator(this, i);
      }
    }
    return const_iterator {this, object_table_.size()};
  }

  const_iterator cbegin() const {
    NIH_ASSERT(this->IsObject());
    return const_iterator {this, 0};
  }
  const_iterator cend() const {
    NIH_ASSERT(this->IsObject());
    return const_iterator {this, object_table_.size()};
  }

  ValueKind get_type() const {
    return this->kind_;
  }

  void Accept(JsonWriter& writer);
};

enum class jError : std::uint8_t {
  kSuccess = 0,
  kInvalidNumber,
  kInvalidArray,
  kInvalidObject,
  kUnexpectedEnd,
  kInvalidString,
  kErrorTrue,
  kErrorFalse,
  kUnknownConstruct
};

using Value = ValueImpl;
/*!
 * \brief JSON document root.  Also is a container for all other derived objects, so it
 * must be the last one to go out of scope when using the JSON tree.  For now only using
 * object as root is supported.
 */
class Document {
  jError err_code_ { jError::kSuccess };
  size_t last_character {0};

  std::vector<size_t> _tree_storage;
  std::vector<std::string::value_type> _data_storage;
  ValueImpl value;

  int32_t n_alive_values_;
  int32_t n_alive_structs_;

 private:
  Document(bool) : value(&_tree_storage, &_data_storage, 0) {
    this->_tree_storage.resize(1);
  }

 public:
  Document() : value(&_tree_storage, &_data_storage, 0) {
    // right now document root must be an object.
    this->_tree_storage.resize(1);
    this->value.SetObject();
  }
  Document(Document const& that) = delete;
  Document(Document&& that) :
      _tree_storage{std::move(that._tree_storage)},
      _data_storage{std::move(that._data_storage)},
      value{ValueImpl{&_tree_storage, &_data_storage, 0}},
      err_code_{that.err_code_},
      last_character{that.last_character} {
        that.value.finalised_ = {true};
        value.object_table_ = std::move(that.value.object_table_);
        value.array_table_ = std::move(that.value.array_table_);
        value.kind_ = that.value.kind_;
        NIH_ASSERT(value.IsObject());
      }

  ~Document() {
    if (!value.finalised_) {
      value.EndObject();
    }
  };

  ValueImpl CreateMember(ConstStringRef key) {
    return this->value.CreateMember(key);
  }

  ValueImpl& GetObject() {
    return value;
  }
  ValueImpl const &GetObject() const {
    return value;
  }

  void Decref() { this->n_alive_values_--; }
  void Incref() { this->n_alive_values_++; }
  void BegStruct() { this->n_alive_structs_++; }
  void EndStruct() { this->n_alive_structs_--; }

  size_t Length() const {
    return this->value.Length();
  }

  jError Errc() const {
    return err_code_;
  }
  std::string ErrStr() const {
    std::string msg;
    switch (err_code_) {
      case jError::kSuccess:
        return "Success";
      case jError::kInvalidNumber:
        msg = "Found invalid floating point number.";
        break;
      case jError::kInvalidArray:
        msg =  "Found invalid array structure.";
      case jError::kInvalidObject:
        msg = "Found invalid object structure.";
        break;
      case jError::kInvalidString:
        msg = "Found invalid string.";
        break;
      case jError::kUnexpectedEnd:
        msg = "Unexpected end.";
        break;
      case jError::kErrorTrue:
        msg = "Error occurred while parsing `true'";
        break;
      case jError::kErrorFalse:
        msg = "Error occurred while parsing `false'";
        break;
      case jError::kUnknownConstruct:
        msg = "Unknown construct.";
        break;
      default:
        LOG(FATAL) << "Unknown error code";
        return "Unknown error code";
    }
    msg += " at character:" + std::to_string(last_character);
    return msg;
  }

  static Document load(StringRef json_str);
  std::string dump();
};

static void print_tree(Span<size_t> tree, size_t beg, size_t end) {
  for (size_t i = beg; i < end; ++i) {
    std::cout << tree[i] << ' ';
  }
  std::cout << std::endl;
}

}  // namespace experimental
}  // namespace nih

#endif  // _NIH_JSON_EXPERIMENTAL_HH_