/* This file is part of NIH.
 *
 * Copyright (c) 2019-2022 Jiaming Yuan <jm.yuan@outlook.com>
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
#include "nih/uri.h"

#include <map>

#include "nih/errors.h"
#include "nih/Logging.h"
#include "nih/strings.hh"

namespace nih {

Uri::Uri(std::string uri)
    : _uri{std::move(uri)}, _is_valid{true}, _code{UriErrorCode::kValid} {
  std::vector<std::string> res;
  split(_uri, &res, [](char c) { return c == ':'; });
  if (res.size() == 0) {
    _is_valid = false;
    _code = UriErrorCode::kEmpty;
  }

  if (res.size() == 1) {
    _scheme_str = "file";
    _code = UriErrorCode::kHost;
    _host_str = res.front();
    _is_valid = true;
  } else {
    auto host = res[1];
    _scheme_str = res[0];
    if (host.size() < 3) {
      _is_valid = false;
      _code = UriErrorCode::kHost;
      return;
    } else {
      _host_str = res[1].substr(2);
    }
  }
}

Uri::Uri(std::string scheme, std::string host, std::string query)
    : _scheme_str{std::move(scheme)}, _host_str{std::move(host)} {}

Uri& Uri::operator=(Uri&& that) {
  std::swap(that._uri, _uri);
  std::swap(that._scheme_str, _scheme_str);
  std::swap(that._host_str, _host_str);
  std::swap(that._is_valid, _is_valid);
  std::swap(that._code, this->_code);
  return *this;
}

Uri& Uri::flush() {
  initialize();
  return *this;
}
}  // namespace nih