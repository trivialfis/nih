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
#ifndef _BASIC_LOGGING_HH_
#define _BASIC_LOGGING_HH_

#include <cstddef>
#include <libgen.h>

#include <cinttypes>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>

#include <nih/config.hh>
#include <nih/errors.hh>
#include <nih/uri.hh>

namespace nih {

class LogImpl;

class Log {
 public:
  enum class ErrorType : uint8_t {
    kFatal = 0,
    kUserError = 1,
    kError = 3,
    kWarning = 4,
    kUser = 5,
    kInfo = 6,
    kDebug = 7
  } error_type_;

 private:
  std::stringstream stream_;

  static ErrorType global_;

  static LogImpl* impl();
  static ErrorType toType(std::string str);

  virtual std::stringstream& fatal();
  virtual std::stringstream& error();
  virtual std::stringstream& userError();

  virtual std::stringstream& user();
  virtual std::stringstream& warning();

  virtual std::stringstream& info();
  virtual std::stringstream& debug();

 public:
  Log();
  virtual ~Log() noexcept(false);

  static void setGlobalVerbosity(std::string value);
  static void setGlobalVerbosity(ErrorType et) { global_ = et; }

  static ErrorType getGlobalVerbosity() { return global_; }
  static ErrorType defaultVerbosity()   { return ErrorType::kUser; }

  static bool shouldLog(ErrorType et)   { return et <= global_; }

  virtual std::stringstream& log(
      std::string msg, ErrorType et=defaultVerbosity());

  static void setThreadName(std::string name);
  static void setUri(ErrorType, UriScheme* scheme);
  static void reset();
};

template <size_t S>
std::string StrBasename(char const (&str)[S]) {
  char vchar[S];
  std::memcpy(vchar, str, sizeof(char) * S);
  return std::string{::basename(vchar)};
}

}  // namespace nih

#define ERROR_FILE_LINE nih::StrBasename(__FILE__) + "(" + std::to_string(__LINE__) + "): "

#if defined(LOG)
#warning "Undefining macro `LOG`'"
#undef LOG
#endif  // defined(LOG)
#define LOG(type) LOG_ ## type

// Unconditional log for errors
#define LOG_FATAL                                                       \
  ::nih::Log().log(ERROR_FILE_LINE, ::nih::Log::ErrorType::kFatal)

#define LOG_ERROR                                                       \
  ::nih::Log().log(ERROR_FILE_LINE, ::nih::Log::ErrorType::kError)

#define LOG_USER_E                                                      \
  ::nih::Log().log("", ::nih::Log::ErrorType::kUserError)

#define LOG_VAR(var) LOG(INFO)  << #var << ":\n" << var

// Conditional log
#define LOG_WARNING                                                     \
  if (::nih::Log::shouldLog(::nih::Log::ErrorType::kWarning))           \
    ::nih::Log().log(ERROR_FILE_LINE, ::nih::Log::ErrorType::kWarning)

#define LOG_USER                                                        \
  if (::nih::Log::shouldLog(::nih::Log::ErrorType::kUser))              \
    ::nih::Log().log(ERROR_FILE_LINE, ::nih::Log::ErrorType::kUser)

#define LOG_INFO                                                        \
  if (::nih::Log::shouldLog(::nih::Log::ErrorType::kInfo))              \
    ::nih::Log().log(ERROR_FILE_LINE, ::nih::Log::ErrorType::kInfo)

#define LOG_DEBUG                                                       \
  if (::nih::Log::shouldLog(::nih::Log::ErrorType::kDebug))             \
    ::nih::Log().log(ERROR_FILE_LINE, ::nih::Log::ErrorType::kDebug)

#define LOG_IF(cond, verbosity)                 \
  if ((cond))                                   \
    LOG(verbosity)                              \


#define NIH_ASSERT(cond)                                                \
  if (NIH_EXPECT(! (cond), false) ) LOG(FATAL) << # cond << "\n"        \


#define NIH_ASSERT_GT(lhs, rhs)                                         \
  if (NIH_EXPECT((lhs) <= (rhs), false))                                \
    LOG(FATAL) << "ASSERT_GT: " << # lhs << ": " << (lhs) << ", " <<    \
        # rhs ": " << (rhs)

#define NIH_ASSERT_GE(lhs, rhs)                                         \
  if (NIH_EXPECT((lhs) < (rhs), false))                                 \
    LOG(FATAL) << "ASSERT_GE: " << # lhs << ": " << (lhs) << ", " <<    \
        # rhs ": " << (rhs)

#define NIH_ASSERT_EQ(lhs, rhs)                                         \
  if (NIH_EXPECT((lhs) != (rhs), false))                                \
    LOG(FATAL) << "ASSERT_EQ: " << # lhs << ": " << (lhs) << ", " <<    \
        # rhs ": " << (rhs)

#define NIH_ASSERT_NE(lhs, rhs)                                         \
  if (NIH_EXPECT((lhs) == (rhs), false))                                \
    LOG(FATAL) << "ASSERT_NE: " << # lhs << ": " << (lhs) << ", " <<    \
        # rhs ": " << (rhs)

#define NIH_ASSERT_LE(lhs, rhs)                                         \
  if (NIH_EXPECT((lhs) > (rhs), false))                                 \
    LOG(FATAL) << "ASSERT_LE: " << # lhs << ": " << (lhs) << ", " <<    \
        # rhs ": " << (rhs)

#define NIH_ASSERT_LT(lhs, rhs)                                         \
  if (NIH_EXPECT((lhs) >= (rhs), false))                                \
    LOG(FATAL) << "ASSERT_LT: " << # lhs << ": " << (lhs) << ", " <<    \
        # rhs ": " << (rhs)

#endif  // _BASIC_LOGGING_HH_