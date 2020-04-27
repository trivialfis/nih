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
#include <signal.h>
#include <unistd.h>

#include <cstdlib>
#include <sstream>

#include "nih/C/logging.h"
#include "api_errors.h"

#include "nih/errors.hh"
#include "nih/logging.hh"
#include "nih/threads.hh"
#include "nih/memory.hh"
#include "nih/uri.hh"

namespace nih {

struct Colorize {
 private:
  bool _is_tty {false};

 public:
  enum Color {
    kRed,
    kYellow,
    kWhite
  };

  explicit Colorize(UriScheme const* uri) {
    // _is_tty = isTTY(*uri);
    _is_tty = true;
  }
  std::string operator()(Color c, std::string const& msg) {
    if (_is_tty) {
      return msg;
    }

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

class LogImpl {
  ThreadStore<std::string> _names;
  std::map<Log::ErrorType, UriScheme*> _out_uris;

  void checkKnownError(Log::ErrorType type) const {
    if (_out_uris.find(type) == _out_uris.cend()) {
      std::cerr << "Unknow Error Type: " << static_cast<uint8_t>(type) << std::endl;
      abort();
    }
  }

 public:
  LogImpl() {
    reset();
  }

  void reset() {
    _out_uris.clear();
    _names.clear();
    using E = Log::ErrorType;
    // Default uris
    _out_uris.emplace(E::kFatal,     &StdErr);
    _out_uris.emplace(E::kUserError, &StdErr);
    _out_uris.emplace(E::kWarning,   &StdErr);
    _out_uris.emplace(E::kError,     &StdErr);
    _out_uris.emplace(E::kUser,      &StdOut);
    _out_uris.emplace(E::kInfo,      &StdOut);
    _out_uris.emplace(E::kDebug,     &StdOut);
  }

  void setThreadName(std::string name) {
    _names.setCurrentThread(std::move(name));
  }
  std::string getThreadName() {
    if (_names.hasValue()) {
      return _names.getCurrentThread();
    }
    return "";
  }

  std::string str() {
    std::string name = this->getThreadName();
    if (name.length() != 0) {
      name = "Thread: " + name + " | ";
    }
    return name;
  }

  void uri(Log::ErrorType type, UriScheme* scheme) {
    checkKnownError(type);
    _out_uris.at(type) = scheme;
    NIH_ASSERT(_out_uris.at(type));
  }

  UriScheme* scheme(Log::ErrorType type) {
    checkKnownError(type);
    return _out_uris.at(type);
  }
};

Log::Log() : error_type_{defaultVerbosity()} {}

Log::ErrorType Log::global_ = Log::defaultVerbosity();

LogImpl* Log::impl() {
  static LogImpl impl;
  return &impl;
}

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

void Log::setThreadName(std::string name) {
  impl()->setThreadName(name);
}

void Log::setUri(ErrorType type, UriScheme* scheme) {
  impl()->uri(type, scheme);
}

void Log::reset() {
  impl()->reset();
}

std::stringstream& Log::fatal() {
  error_type_ = ErrorType::kFatal;
  stream_ << impl()->str() <<
      Colorize{impl()->scheme(ErrorType::kFatal)}(Colorize::kRed, "[FATAL]") << ": ";
  return stream_;
}

std::stringstream& Log::error() {
  error_type_ = ErrorType::kError;
  stream_ << impl()->str() <<
      Colorize{impl()->scheme(ErrorType::kError)}(Colorize::kRed, "[ERROR]") << ": ";
  return stream_;
}

std::stringstream& Log::userError() {
  error_type_ = ErrorType::kUserError;
  stream_ << impl()->str() <<
      Colorize{impl()->scheme(ErrorType::kUser)}(Colorize::kRed, "[USER ERROR]") << ": ";
  return stream_;
}

std::stringstream& Log::user() {
  error_type_ = ErrorType::kUser;
  stream_ << impl()->str();
  return stream_;
}

std::stringstream& Log::warning() {
  error_type_ = ErrorType::kWarning;
  stream_ << impl()->str() <<
      Colorize{impl()->scheme(ErrorType::kWarning)}(Colorize::kYellow, "[WARNING]") << ": ";
  return stream_;
}

std::stringstream& Log::info() {
  error_type_ = ErrorType::kInfo;
  stream_ << impl()->str() <<
      Colorize{impl()->scheme(ErrorType::kInfo)}(Colorize::kWhite, "[INFO]") << ": ";
  return stream_;
}

std::stringstream& Log::debug() {
  error_type_ = ErrorType::kDebug;
  stream_ << impl()->str() << "[DEBUG]: ";
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
  stream_ << '\n';
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
      impl()->scheme(ErrorType::kWarning)->write(stream_.str()).flush();
      break;
    case ErrorType::kError:
      impl()->scheme(ErrorType::kError)->write(stream_.str()).flush();
      break;
    case ErrorType::kUser:
      impl()->scheme(ErrorType::kUser)->write(stream_.str()).flush();
      break;
    case ErrorType::kInfo:
      impl()->scheme(ErrorType::kInfo)->write(stream_.str()).flush();
      break;
    case ErrorType::kDebug:
      impl()->scheme(ErrorType::kDebug)->write(stream_.str()).flush();
      break;
    default:
      LOG(FATAL) << "Unknow verbosity: " << static_cast<int>(error_type_);
  }
}

class SignalsHandling {
  bool _success {true};

 public:
  SignalsHandling() {
    // Signal snippet borrowed from backward-cpp
    std::vector<int32_t> _posix_signals {
      // Signals for which the default action is "Core".
      SIGABRT, // Abort signal from abort(3)
      SIGBUS,  // Bus error (bad memory access)
      SIGFPE,  // Floating point exception
      SIGILL,  // Illegal Instruction
      SIGIOT,  // IOT trap. A synonym for SIGABRT
      SIGQUIT, // Quit from keyboard
      SIGSEGV, // Invalid memory reference
      SIGSYS,  // Bad argument to routine (SVr4)
      SIGTRAP, // Trace/breakpoint trap
      SIGXCPU, // CPU time limit exceeded (4.2BSD)
      SIGXFSZ, // File size limit exceeded (4.2BSD)
    };


    for (size_t i = 0; i < _posix_signals.size(); ++i) {
      struct sigaction action;
      memset(&action, 0, sizeof action);
      action.sa_flags =
          static_cast<int>(SA_SIGINFO | SA_ONSTACK | SA_NODEFER | SA_RESETHAND);
      sigfillset(&action.sa_mask);
      sigdelset(&action.sa_mask, _posix_signals[i]);
      action.sa_sigaction = &sigHandler;
      int r = sigaction(_posix_signals[i], &action, nullptr);
      if (r < 0) {
        _success = false;
      }
    }
  }
  static void sigHandler(int signo, siginfo_t *info, void *_ctx) {
    handleSignal(signo, info, _ctx);
    raise(info->si_signo);

    // terminate the process immediately.
    puts("watf? exit");
    _exit(EXIT_FAILURE);
  }
  static void handleSignal(int32_t, siginfo_t* info, void* ctx) {
    auto *uctx = static_cast<ucontext_t *>(ctx);
    StackTrace st;
    std::cerr << st.str();
  }
};

SignalsHandling sh;

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