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
#ifndef _JSON_HH_
#define _JSON_HH_

#include <iostream>
#include <istream>
#include <string>

#include <map>
#include <memory>
#include <vector>

namespace nih {
namespace json {

class Json;
class JsonWriter;

class Value {
 public:
  /*!\brief Simplified implementation of LLVM RTTI. */
  enum class ValueKind {
    String,
    Number,
    Object,  // std::map
    Array,   // std::vector
    Boolean,
    Null
  };

  Value(ValueKind _kind) : kind_{_kind} {}

  ValueKind type() const { return kind_; }
  virtual ~Value() = default;

  virtual void save(JsonWriter* stream) = 0;

  virtual Json& operator[](std::string const & key) = 0;
  virtual Json& operator[](int ind) = 0;

  virtual bool operator==(Value const& rhs) const = 0;
  virtual Value& operator=(Value const& rhs) = 0;

  std::string typeStr() const;

 private:
  ValueKind kind_;
};

template <typename T>
bool IsA(Value const* value) {
  return T::isClassOf(value);
}

template <typename T, typename U>
T* Cast(U* value) {
  if (IsA<T>(value)) {
    return dynamic_cast<T*>(value);
  } else {
    throw std::runtime_error(
        "Invalid cast, from " + value->typeStr() + " to " + T().typeStr());
  }
}

class JsonString : public Value {
  std::string str_;
 public:
  JsonString() : Value(ValueKind::String) {}
  JsonString(std::string const& str) :
      Value(ValueKind::String), str_{str} {}
  JsonString(std::string&& str) :
      Value(ValueKind::String), str_{std::move(str)} {}

  void save(JsonWriter* stream) override;

  Json& operator[](std::string const & key) override;
  Json& operator[](int ind) override;

  std::string const& getString() &&      { return str_; }
  std::string const& getString() const & { return str_; }
  std::string&       getString()       & { return str_;}

  bool operator==(Value const& rhs) const override;
  Value& operator=(Value const& rhs) override;

  static bool isClassOf(Value const* value) {
    return value->type() == ValueKind::String;
  }
};

class JsonArray : public Value {
  std::vector<Json> vec_;

 public:
  JsonArray() : Value(ValueKind::Array) {}
  JsonArray(std::vector<Json>&& arr) :
      Value(ValueKind::Array), vec_{std::move(arr)} {}
  JsonArray(std::vector<Json> const& arr) :
      Value(ValueKind::Array), vec_{arr} {}

  void save(JsonWriter* stream) override;

  Json& operator[](std::string const & key) override;
  Json& operator[](int ind) override;

  std::vector<Json> const& getArray() &&      { return vec_; }
  std::vector<Json> const& getArray() const & { return vec_; }
  std::vector<Json>&       getArray()       & { return vec_; }

  bool operator==(Value const& rhs) const override;
  Value& operator=(Value const& rhs) override;

  static bool isClassOf(Value const* value) {
    return value->type() == ValueKind::Array;
  }
};

class JsonObject : public Value {
  std::map<std::string, Json> object_;

 public:
  JsonObject() : Value(ValueKind::Object) {}
  JsonObject(std::map<std::string, Json> object);

  void save(JsonWriter* writer) override;

  Json& operator[](std::string const & key) override;
  Json& operator[](int ind) override;

  std::map<std::string, Json> const& getObject() &&      { return object_; }
  std::map<std::string, Json> const& getObject() const & { return object_; }
  std::map<std::string, Json> &      getObject() &       { return object_; }

  bool operator==(Value const& rhs) const override;
  Value& operator=(Value const& rhs) override;

  static bool isClassOf(Value const* value) {
    return value->type() == ValueKind::Object;
  }
  virtual ~JsonObject() = default;
};

class JsonNumber : public Value {
  double number_;

 public:
  JsonNumber() : Value(ValueKind::Number) {}
  JsonNumber(double value) : Value(ValueKind::Number) {
    number_ = value;
  }

  void save(JsonWriter* stream) override;

  Json& operator[](std::string const & key) override;
  Json& operator[](int ind) override;

  double const& getNumber() &&      { return number_; }
  double const& getNumber() const & { return number_; }
  double&       getNumber()       & { return number_; }

  bool operator==(Value const& rhs) const override;
  Value& operator=(Value const& rhs) override;

  static bool isClassOf(Value const* value) {
    return value->type() == ValueKind::Number;
  }
};

class JsonNull : public Value {
 public:
  JsonNull() : Value(ValueKind::Null) {}
  JsonNull(std::nullptr_t) : Value(ValueKind::Null) {}

  void save(JsonWriter* stream) override;

  Json& operator[](std::string const & key) override;
  Json& operator[](int ind) override;

  bool operator==(Value const& rhs) const override;
  Value& operator=(Value const& rhs) override;

  static bool isClassOf(Value const* value) {
    return value->type() == ValueKind::Null;
  }
};

/*! \brief Describes both true and false. */
class JsonBoolean : public Value {
  bool boolean_;
 public:
  JsonBoolean() : Value(ValueKind::Boolean) {}
  // Ambigious with JsonNumber.
  template <typename Bool,
            typename std::enable_if<
              std::is_same<Bool, bool>::value ||
              std::is_same<Bool, bool const>::value>::type* = nullptr>
  JsonBoolean(Bool value) :
      Value(ValueKind::Boolean), boolean_{value} {}

  void save(JsonWriter* writer) override;

  Json& operator[](std::string const & key) override;
  Json& operator[](int ind) override;

  bool const& getBoolean() &&      { return boolean_; }
  bool const& getBoolean() const & { return boolean_; }
  bool&       getBoolean()       & { return boolean_; }

  bool operator==(Value const& rhs) const override;
  Value& operator=(Value const& rhs) override;

  static bool isClassOf(Value const* value) {
    return value->type() == ValueKind::Boolean;
  }
};

/*!
 * \brief Data structure representing JSON format.
 *
 * Limitation:  UTF-8 is not properly supported.  Code points above ASCII are
 *              invalid.
 *
 * Examples:
 *
 * \code
 *   // Create a JSON object.
 *   json::Json object = json::Object();
 *   // Assign key "key" with a JSON string "Value";
 *   object["key"] = Json::String("Value");
 *   // Assign key "arr" with a empty JSON Array;
 *   object["arr"] = Json::Array();
 * \endcode
 */
class Json {
  friend JsonWriter;
  void save(JsonWriter* writer) {
    this->ptr_->save(writer);
  }

 public:
  /*! \brief Load a Json file from stream. */
  static Json load(std::istream* stream);
  /*! \brief Dump json into stream. */
  static void dump(Json json, std::ostream* stream);

  Json() : ptr_{new JsonNull} {}

  // number
  explicit Json(JsonNumber number) : ptr_{new JsonNumber(number)} {}
  Json& operator=(JsonNumber number) {
    ptr_.reset(new JsonNumber(std::move(number)));
    return *this;
  }
  // array
  explicit Json(JsonArray list) :
      ptr_{new JsonArray(std::move(list))}{}
  Json& operator=(JsonArray array) {
    ptr_.reset(new JsonArray(std::move(array)));
    return *this;
  }
  // object
  explicit Json(JsonObject object) :
      ptr_{new JsonObject(std::move(object))} {}
  Json& operator=(JsonObject object) {
    ptr_.reset(new JsonObject(std::move(object)));
    return *this;
  }
  // string
  explicit Json(JsonString str) :
      ptr_{new JsonString(std::move(str))} {}
  Json& operator=(JsonString str) {
    ptr_.reset(new JsonString(std::move(str)));
    return *this;
  }
  // bool
  explicit Json(JsonBoolean boolean) :
      ptr_{new JsonBoolean(std::move(boolean))} {}
  Json& operator=(JsonBoolean boolean) {
    ptr_.reset(new JsonBoolean(std::move(boolean)));
    return *this;
  }
  // null
  explicit Json(JsonNull null) :
      ptr_{new JsonNull(std::move(null))} {}
  Json& operator=(JsonNull null) {
    ptr_.reset(new JsonNull(std::move(null)));
    return *this;
  }

  // copy
  Json(Json const& other) : ptr_{other.ptr_} {}
  Json& operator=(Json const& other);
  // move
  Json(Json&& other) : ptr_{std::move(other.ptr_)} {}
  Json& operator=(Json&& other) {
    ptr_ = std::move(other.ptr_);
    return *this;
  }

  /*! \brief Index Json object with a std::string, used for Json Object. */
  Json& operator[](std::string const & key) const { return (*ptr_)[key]; }
  /*! \brief Index Json object with int, used for Json Array. */
  Json& operator[](int ind)                 const { return (*ptr_)[ind]; }

  /*! \Brief Return the reference to stored Json value. */
  Value const& getValue() const & { return *ptr_;}
  Value const& getValue() &&      { return *ptr_; }
  Value&       getValue() &       { return *ptr_; }

  bool operator==(Json const& rhs) const {
    return *ptr_ == *(rhs.ptr_);
  }

 private:
  std::shared_ptr<Value> ptr_;
};

namespace detail {

// Number
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonNumber>::value>::type* = nullptr>
double& getImpl(T& val) {
  return val.getNumber();
}
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonNumber const>::value>::type* = nullptr>
double const& getImpl(T& val) {
  return val.getNumber();
}

// String
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonString>::value>::type* = nullptr>
std::string& getImpl(T& val) {
  return val.getString();
}
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonString const>::value>::type* = nullptr>
std::string const& getImpl(T& val) {
  return val.getString();
}

// Boolean
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonBoolean>::value>::type* = nullptr>
bool& getImpl(T& val) {
  return val.getBoolean();
}
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonBoolean const>::value>::type* = nullptr>
bool const& getImpl(T& val) {
  return val.getBoolean();
}

// Array
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonArray>::value>::type* = nullptr>
std::vector<Json>& getImpl(T& val) {
  return val.getArray();
}
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonArray const>::value>::type* = nullptr>
std::vector<Json> const& getImpl(T& val) {
  return val.getArray();
}

// Object
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonObject>::value>::type* = nullptr>
std::map<std::string, Json>& getImpl(T& val) {
  return val.getObject();
}
template <typename T,
          typename std::enable_if<
            std::is_same<T, json::JsonObject const>::value>::type* = nullptr>
std::map<std::string, Json> const& getImpl(T& val) {
  return val.getObject();
}

}  // namespace detail

/*!
 * \brief Get Json value.
 *
 * \tparam T One of the Json value type.
 *
 * \param json
 * \return Value contained in Json object of type T.
 */
template <typename T, typename U>
auto get(U& json) -> decltype(detail::getImpl(*Cast<T>(&json.getValue())))& {
  auto& value = *Cast<T>(&json.getValue());
  return detail::getImpl(value);
}

using Object = JsonObject;
using Array = JsonArray;
using Number = JsonNumber;
using Boolean = JsonBoolean;
using String = JsonString;
using Null = JsonNull;

}  // namespace json
}  // namespace nih

#endif  // _JSON_HH_