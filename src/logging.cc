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
#include <sstream>

#include "nih/C/logging.h"
#include "api_errors.h"

#include "nih/errors.hh"
#include "nih/logging.hh"

namespace nih {

struct Colorize {
  enum Color {
    kRed,
    kYellow,
    kWhite
  };
  std::string operator()(Color c, std::string const& msg) {
    std::string reset = "\u001b[0m";
    switch (c) {
      case kRed:
        return "\u001b[31m" + msg + reset;
      case kYellow:
        return "\u001b[33m" + msg + reset;
      case kWhite:
        return "\u001b[37m" + msg + reset;
      default:
        throw NIHError("Unknown color.");
    }
    return msg;
  }
};
Log::ErrorType Log::global_ = Log::defaultVerbosity();

Log::ErrorType Log::toType(std::string str) {
  if (str == "Fatal") {
    return ErrorType::kFatal;
  } else if (str == "Error") {
    return ErrorType::kError;
  } else if (str == "UserError") {
    return ErrorType::kUserError;
  } else if (str == "Warning") {
    return ErrorType::kWarning;
  } else if (str == "User") {
    return ErrorType::kUser;
  } else if (str == "Info") {
    return ErrorType::kInfo;
  } else if (str == "Debug") {
    return ErrorType::kDebug;
  } else {
    LOG(WARNING) << "Unknown verbosity: " << str;
    return defaultVerbosity();
  }
}

void Log::setGlobalVerbosity(std::string value) {
  auto type = toType(value);
  setGlobalVerbosity(type);
}

std::stringstream& Log::fatal() {
  error_type_ = ErrorType::kFatal;
  stream_ << Colorize{}(Colorize::kRed, "[FATAL]") << ": ";
  return stream_;
}

std::stringstream& Log::error() {
  error_type_ = ErrorType::kError;
  stream_ << Colorize{}(Colorize::kRed, "[ERROR]") << ": ";
  return stream_;
}

std::stringstream& Log::userError() {
  error_type_ = ErrorType::kUserError;
  stream_ << Colorize{}(Colorize::kRed, "[USER ERROR]") << ": ";
  return stream_;
}

std::stringstream& Log::user() {
  error_type_ = ErrorType::kUser;
  return stream_;
}

std::stringstream& Log::warning() {
  error_type_ = ErrorType::kWarning;
  stream_ << Colorize{}(Colorize::kYellow, "[WARNING]") << ": ";
  return stream_;
}

std::stringstream& Log::info() {
  error_type_ = ErrorType::kInfo;
  stream_ << Colorize{}(Colorize::kWhite, "[INFO]") << ": ";
  return stream_;
}

std::stringstream& Log::debug() {
  error_type_ = ErrorType::kDebug;
  stream_ << "[DEBUG]: ";
  return stream_;
}

std::stringstream& Log::log(std::string msg, ErrorType et) {
  error_type_ = et;

  switch (et) {
    case ErrorType::kFatal:
      fatal() << msg;
      return stream_;
    case ErrorType::kError:
      error() << msg;
      return stream_;
    case ErrorType::kUserError:
      userError() << msg;
      return stream_;
    default:
      break;
  }
  if (!shouldLog(et)) {
    return stream_;
  }
  switch (et){
    case ErrorType::kWarning:
      warning() << msg;
      break;
    case ErrorType::kUser:
      user() << msg;
      break;
    case ErrorType::kInfo:
      info() << msg;
      break;
    case ErrorType::kDebug:
      debug() << msg;
      break;
    default:
      LOG(FATAL) << "Unknow verbosity: " << static_cast<int>(et);
  }
  return stream_;
}

Log::~Log() noexcept(false) {
  switch (error_type_){
    // throw
    case ErrorType::kFatal:
      throw FatalError(stream_.str() + "\n");
      break;
    case ErrorType::kUserError:
      throw RecoverableError(stream_.str() + "\n");
      break;
    default:
      break;
  }
  if (!shouldLog(error_type_)) {
    return;
  }
  switch (error_type_) {
    // non throw
    case ErrorType::kWarning:
      std::cerr << stream_.str() << std::endl;
      break;
    case ErrorType::kError:
      std::cerr << stream_.str() << std::endl;
      break;
    case ErrorType::kUser:
      std::cout << stream_.str() << std::endl;
      break;
    case ErrorType::kInfo:
      std::cout << stream_.str() << std::endl;
      break;
    case ErrorType::kDebug:
      std::cout << stream_.str() << std::endl;
      break;
    default:
      LOG(FATAL) << "Unknow verbosity: " << static_cast<int>(error_type_);
  }
}

}  // namespace nih

NIH_API NIH_ErrCode nih_log(char const* msg, int verbosity) {
  C_API_BEG()
    NIH_ASSERT_GE(verbosity, 0);
    int maximum_verbosity = static_cast<int>(::nih::Log::ErrorType::kDebug);
    NIH_ASSERT_LE(verbosity, maximum_verbosity);
    ::nih::Log::ErrorType et = static_cast<::nih::Log::ErrorType>(verbosity);
    ::nih::Log().log(std::string{msg}, et);
  C_API_END()
}

NIH_API NIH_ErrCode nih_logDefault(char const* msg) {
  return nih_log(msg, static_cast<int>(::nih::Log::defaultVerbosity()));
}

NIH_API NIH_ErrCode nih_setLogVerbosity(char const* msg) {
  C_API_BEG()
    ::nih::Log::setGlobalVerbosity(msg);
  C_API_END()
}