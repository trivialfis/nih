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
    auto v = ptr_->GetMemberByIndex(index_);
    return v;
  }
  value_type operator[](difference_type i) const {
    NIH_ASSERT(ptr_);
    NIH_ASSERT_LT(i + index_, ptr_->Length());
    ptr_->GetMemberByIndex(i + index_);
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
  void Resize(size_t n) { this->storage_ref_->resize(n); }

 public:
  StorageView(std::vector<T> *storage_ref) : storage_ref_{storage_ref} {}
  size_t Top() const { return storage_ref_->size(); }
  Span<T> Access() const { return Span<T>{*storage_ref_}; }
  void Expand(size_t n) { this->Resize(storage_ref_->size() + n); }

  template <typename V>
  void Push(V* value, size_t n) {
    size_t old = storage_ref_->size();
    this->Expand(sizeof(V) / sizeof(T) * n);
    std::memcpy(storage_ref_->data() + old, value, n * sizeof(V));
  }
  std::vector<T> *Data() const { return storage_ref_; }
};

class JsonWriter;
class Document;

template <typename Container>
class ValueImpl {
 protected:
  // Using inheritence is not idea as Document would go out of scope before this base
  // class, while memory buffer is held in Document;
  friend Document;
  using ValueImplT = ValueImpl;

  Container* handler_;
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
  };

  using iterator = ElemIterator<ValueImplT, false>;
  using const_iterator = ElemIterator<ValueImplT, true>;

  struct StringStorage {
    size_t beg;
    size_t end;
  };

 protected:
  // A `this` pointer pointing to JSON tree, with type information written in first 3 bits.
  size_t self_ {0};
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
  ValueImpl(Container *doc) : handler_{doc}, self_{0} {}

  ValueImpl(Container *doc, size_t tree_beg)
      : handler_{doc}, self_{tree_beg}, kind_{ValueKind::kNull} {}

  // ValueImpl knows how to construct itself from data kind and a pointer to
  // its storage
  ValueImpl(Container *doc, ValueKind kind, size_t self)
      : kind_{kind}, self_{self}, is_view_{true}, handler_{doc} {
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
      Span<size_t> tree = handler_->Tree().Access();
      size_t table_offset = JsonTypeHandler::GetOffset(tree[self_]);
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
      Span<size_t> tree = handler_->Tree().Access();
      size_t table_offset = JsonTypeHandler::GetOffset(tree[self_]);
      size_t length = tree[table_offset];
      array_table_.resize(length);
      auto tree_ptr = table_offset + 1;
      std::memcpy(array_table_.data(), tree.data() + tree_ptr,
                  length * sizeof(size_t));
      break;
    }
    case ValueKind::kNull: {
      this->kind_ = ValueKind::kNull;
      break;
    }
    case ValueKind::kString: {
      this->kind_ = ValueKind::kString;
      Span<size_t> tree = handler_->Tree().Access();
      auto storage_ptr = JsonTypeHandler::GetOffset(tree[self_]);
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
    Span<size_t> tree = handler_->Tree().Access();
    auto const current_data_pointer = handler_->Data().Top();
    tree[this->self_] =
        JsonTypeHandler::MakeTypedOffset(current_data_pointer, kind);
    handler_->Data().Push(&value, 1);
  }

  template <typename T>
  T GetNumber(ValueKind kind) const {
    CheckType(kind);
    Span<size_t> tree = handler_->Tree().Access();
    auto data_ptr = JsonTypeHandler::GetOffset(tree[self_]);
    auto data = handler_->Data().Access();
    T value { T() };
    std::memcpy(&value, data.data() + data_ptr, sizeof(value));
    return value;
  }

 public:
   ValueImpl(ValueImpl const &that) // = delete
       : self_{that.self_}, handler_{that.handler_},
         object_table_{that.object_table_}, array_table_{that.array_table_},
         kind_{that.kind_}, finalised_{that.finalised_}, is_view_{
                                                             that.is_view_} {}
   ValueImpl(ValueImpl &&that)
       : self_{that.self_}, handler_{that.handler_},
         object_table_{std::move(that.object_table_)},
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
    StorageView<size_t> tree_storage = handler_->Tree();
    auto current_tree_pointer = tree_storage.Top();
    auto tree = tree_storage.Access();
    tree[this->self_] = JsonTypeHandler::MakeTypedOffset(
        current_tree_pointer, ValueKind::kString);

    tree_storage.Expand(2);
    tree = tree_storage.Access();

    StorageView<char> data_storage = handler_->Data();
    auto current_data_pointer = data_storage.Top();

    data_storage.Expand(string.size());
    auto data = data_storage.Access();
    std::memcpy(data.data() + current_data_pointer, string.data(), string.size());

    string_storage.beg = current_data_pointer;
    string_storage.end = current_data_pointer + string.size();

    tree[current_tree_pointer] = string_storage.beg;
    tree[current_tree_pointer + 1] = string_storage.end;
    return *this;
  }

  ConstStringRef GetString() const {
    CheckType(ValueKind::kString);
    auto data = handler_->Data().Access();
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
    Span<size_t> tree = handler_->Tree().Access();
    tree[self_] = JsonTypeHandler::MakeTypedOffset(kElementEnd,
                                                         ValueKind::kArray);
    array_table_.resize(length, kElementEnd);
    finalised_ = false;
    return *this;
  }

  ValueImpl GetArrayElem(size_t index) {
    CheckType(ValueKind::kArray);
    NIH_ASSERT_LT(index, array_table_.size());
    StorageView<size_t> tree_storage = handler_->Tree();
    auto tree = tree_storage.Access();
    ValueKind kind;
    if (array_table_[index] == kElementEnd) {
      auto value = ValueImpl {handler_, tree_storage.Top()};
      array_table_[index] = tree_storage.Top();
      tree_storage.Expand(1);
      return value;
    } else {
      kind = JsonTypeHandler::GetType(tree[array_table_[index]]);
      ValueImpl value(handler_, kind, array_table_[index]);
      return value;
    }
  }

  ValueImpl SetArray() {
    array_table_.reserve(8);
    this->SetArray(0);
    return *this;
  }
  ValueImpl CreateArrayElem() {
    CheckType(ValueKind::kArray);
    StorageView<size_t> tree_storage = handler_->Tree();
    auto value = ValueImpl {handler_, tree_storage.Top()};
    array_table_.push_back(tree_storage.Top());
    tree_storage.Expand(1);
    return value;
  }

  ValueImpl SetObject() {
    // initialize the value here.
    InitializeType(ValueKind::kObject);
    auto tree = handler_->Tree().Access();
    tree[self_] =
        JsonTypeHandler::MakeTypedOffset(self_, ValueKind::kObject);
    finalised_ = false;
    return *this;
  }

  ValueImpl CreateMember(ConstStringRef key) {
    CheckType(ValueKind::kObject);
    StorageView<size_t> tree_storage = handler_->Tree();
    size_t const current_tree_pointer = tree_storage.Top();
    // allocate space for object element.
    tree_storage.Expand(sizeof(ObjectElement) / sizeof(size_t) + 1);
    auto tree = tree_storage.Access();

    auto data_storage = handler_->Data();
    size_t const current_data_pointer =  data_storage.Top();
    // allocate space for key
    data_storage.Expand(key.size() * sizeof(ConstStringRef::value_type));
    auto data = data_storage.Access();
    std::copy(key.cbegin(), key.cend(), data.begin() + current_data_pointer);

    ObjectElement elem;
    elem.key_begin = current_data_pointer;
    elem.key_end = current_data_pointer + key.size();
    elem.value = tree_storage.Top() - 1;

    object_table_.push_back(elem);

    auto value = ValueImpl{handler_, elem.value};
    return value;
  }

  void EndObject() {
    CheckType(ValueKind::kObject);
    NIH_ASSERT(!finalised_);
    StorageView<size_t> tree_storage = handler_->Tree();
    size_t current_tree_pointer = tree_storage.Top();
    auto table_begin = current_tree_pointer;

    tree_storage.Expand(object_table_.size() * 3 + 1);  // +1 length
    auto tree = tree_storage.Access();
    tree[current_tree_pointer] = object_table_.size();
    current_tree_pointer += 1;

    for (size_t i = 0; i < object_table_.size(); ++i) {
      tree[current_tree_pointer] = object_table_[i].key_begin;
      tree[current_tree_pointer + 1] = object_table_[i].key_end;
      tree[current_tree_pointer + 2] = object_table_[i].value;
      current_tree_pointer += 3;
    }

    tree[self_] = JsonTypeHandler::MakeTypedOffset(
        table_begin, ValueKind::kObject);
    finalised_ = true;
  }

  void EndArray() {
    CheckType(ValueKind::kArray);
    NIH_ASSERT(!finalised_);
    StorageView<size_t> tree_storage = handler_->Tree();
    size_t current_tree_pointer = tree_storage.Top();
    auto table_begin = current_tree_pointer;

    tree_storage.Expand(array_table_.size() + 1);
    auto tree = tree_storage.Access();
    tree[current_tree_pointer] = array_table_.size();
    current_tree_pointer += 1;

    std::copy(array_table_.cbegin(), array_table_.cend(),
              tree.begin() + current_tree_pointer);

    tree[self_] = JsonTypeHandler::MakeTypedOffset(
        table_begin, ValueKind::kArray);
    finalised_ = true;
  }

  ValueImplT GetMemberByIndex(size_t index) const {
    CheckType(ValueKind::kObject);
    NIH_ASSERT_LT(index, object_table_.size());
    StorageView<size_t> tree_storage = handler_->Tree();
    auto tree = tree_storage.Access();
    auto elem = object_table_[index];
    ValueKind kind;
    size_t offset_in_tree;
    std::tie(kind, offset_in_tree) =
        JsonTypeHandler::GetTypeOffset(tree[elem.value]);
    ValueImpl value{handler_, kind, elem.value};
    return value;
  }

  const_iterator FindMemberByKey(ConstStringRef key) const {
    CheckType(ValueKind::kObject);
    Span<size_t> tree = handler_->Tree().Access();
    auto data = handler_->Data().Access();
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
    CheckType(ValueKind::kObject);
    return const_iterator {this, 0};
  }
  const_iterator cend() const {
    CheckType(ValueKind::kObject);
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

  using Value = ValueImpl<Document>;
  Value value;

  int32_t n_alive_values_;
  int32_t n_alive_structs_;

  StorageView<size_t> Tree() {
    return StorageView<size_t>{&_tree_storage};
  }
  StorageView<char> Data() {
    return StorageView<char> {&_data_storage};
  }
  friend Value;

 private:
  Document(bool) : value(this) {
    this->_tree_storage.resize(1);
  }

 public:
  Document() : value(this) {
    // right now document root must be an object.
    this->_tree_storage.resize(1);
    this->value.SetObject();
  }
  Document(Document const& that) = delete;
  Document(Document&& that) :
      _tree_storage{std::move(that._tree_storage)},
      _data_storage{std::move(that._data_storage)},
      value{ValueImpl<Document>{this}},
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

  Value CreateMember(ConstStringRef key) {
    return this->value.CreateMember(key);
  }

  Value& GetObject() {
    return value;
  }
  Value const &GetObject() const {
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

  static Document Load(StringRef json_str);
  std::string Dump();
};

using Json = ValueImpl<Document>;
}  // namespace experimental
}  // namespace nih

#endif  // _NIH_JSON_EXPERIMENTAL_HH_