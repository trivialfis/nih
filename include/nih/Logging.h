/*
 * Copyright 2021 The Nih Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 */
#ifndef NIH_LOGGING_H_
#define NIH_LOGGING_H_

#include <nih/Intrinsics.h>
#include <nih/StringRef.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

namespace nih {

class Logger;

/**
 * A Python like logging module.
 */
class Logging {
 public:
  static Logger &getLogger(std::string str);
};

class Logger {
  std::string _name;

 public:
  enum Verbosity {
    kFatal = 0,
    kWarning = 1,
    kInfo = 2,
    kDebug = 3,
  };

 protected:
  class Stream {
    std::stringstream _ss;
    Verbosity _v;
    bool _ignore;

    static void defaultHandler(char const *msg);
    std::function<decltype(defaultHandler)> _handler;

    std::function<decltype(defaultHandler)> getHandler() const {
      // fatal error should always be handled inside C++ library due to stack
      // unwinding.
      if (_handler && _v != kFatal) {
        return _handler;
      }
      return defaultHandler;
    }

   public:
    explicit Stream(Verbosity v, std::function<void(char const *)> handler, bool ignore)
        : _v{v}, _ignore{ignore}, _handler{std::move(handler)} {}
    Stream(Stream &&s)
        : _ss{std::move(s._ss)}, _v{s._v}, _ignore{s._ignore}, _handler{s._handler} {
      s._ignore = true;
    }

    ~Stream() noexcept(false);

    template <typename T>
    Stream &operator<<(T const &v) {
      if (NIH_UNLIKELY(!_ignore)) {
        _ss << v;
      }
      return *this;
    }
  };

 private:
  Verbosity _verbosity{kWarning};
  std::function<void(char const *)> _handler{nullptr};

 public:
  explicit Logger(ConstStringRef name) : _name{name} {}
  Logger(Logger const &that) = delete;

  Stream fatal() {
    return std::move(Stream(kFatal, _handler, false)
                     << "[" << this->_name << "] fatal: ");
  }
  Stream warn() {
    return std::move(Stream(kWarning, _handler, _verbosity < kWarning)
                     << "[" << this->_name << "] warning: ");
  }
  Stream info() {
    return std::move(Stream(kInfo, _handler, _verbosity < kInfo)
                     << "[" << this->_name << "] info: ");
  }
  Stream debug() {
    return std::move(Stream(kDebug, _handler, _verbosity < kDebug)
                     << "[" << this->_name << "] debug: ");
  }

  void verbosity(Verbosity v) { this->_verbosity = v; }
  Verbosity verbosity() { return this->_verbosity; }

  void registerHandler(std::function<void(char const *)> handler) {
    _handler = handler;
  }

  Logger &fork(std::string name) {
    auto &logger = Logging::getLogger(this->_name + "::" + name);
    logger._verbosity = this->_verbosity;
    logger._handler = this->_handler;
    return logger;
  }
};

inline Logger &Logging::getLogger(std::string str) {
  static thread_local std::map<std::string, std::shared_ptr<Logger>> _loggers;
  if (_loggers.find(str) == _loggers.cend()) {
    _loggers[str] = std::make_shared<Logger>(str);
  }
  return *_loggers[str];
}

constexpr static char const *kLoggerName = "nih";

#define NIH_LOG_FATAL ::nih::Logging::getLogger(::nih::kLoggerName).fatal()

#define NIH_LOG_WARN ::nih::Logging::getLogger(::nih::kLoggerName).warn()

#define NIH_LOG_INFO ::nih::Logging::getLogger(::nih::kLoggerName).info()

#define NIH_LOG_DEBUG ::nih::Logging::getLogger(::nih::kLoggerName).debug()

#define LOG(severity) NIH_LOG_##severity

template <typename U, typename V>
void LogBinaryOp(U const &l, V const &r, ConstStringRef op) {
  LOG(FATAL) << "Condition " << l << " " << op << " " << r << " .";
}

#define NIH_ASSERT_T(cond)                 \
  do {                                     \
    if (NIH_UNLIKELY(!(cond))) {           \
      LOG(FATAL) << #cond << " is false."; \
    }                                      \
  } while (0)

#define NIH_ASSERT_F(cond)                \
  do {                                    \
    if (NIH_UNLIKELY(cond)) {             \
      LOG(FATAL) << #cond << " is true."; \
    }                                     \
  } while (0)

#define NIH_ASSERT_GT(lhs, rhs)     \
  if (NIH_UNLIKELY((lhs) <= (rhs))) \
  LOG(FATAL) << "ASSERT_GT: " << #lhs << ": " << (lhs) << ", " << #rhs ": " << (rhs)

#define NIH_ASSERT_GE(lhs, rhs)    \
  if (NIH_UNLIKELY((lhs) < (rhs))) \
  LOG(FATAL) << "ASSERT_GE: " << #lhs << ": " << (lhs) << ", " << #rhs ": " << (rhs)

#define NIH_ASSERT_EQ(lhs, rhs)     \
  if (NIH_UNLIKELY((lhs) != (rhs))) \
  LOG(FATAL) << "ASSERT_EQ: " << #lhs << ": " << (lhs) << ", " << #rhs ": " << (rhs)

#define NIH_ASSERT_NE(lhs, rhs)     \
  if (NIH_UNLIKELY((lhs) == (rhs))) \
  LOG(FATAL) << "ASSERT_NE: " << #lhs << ": " << (lhs) << ", " << #rhs ": " << (rhs)

#define NIH_ASSERT_LE(lhs, rhs)    \
  if (NIH_UNLIKELY((lhs) > (rhs))) \
  LOG(FATAL) << "ASSERT_LE: " << #lhs << ": " << (lhs) << ", " << #rhs ": " << (rhs)

#define NIH_ASSERT_LT(lhs, rhs)     \
  if (NIH_UNLIKELY((lhs) >= (rhs))) \
  LOG(FATAL) << "ASSERT_LT: " << #lhs << ": " << (lhs) << ", " << #rhs ": " << (rhs)
}  // namespace nih

#endif  // NIH_LOGGING_H_
