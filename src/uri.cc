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
#include <map>

#include "nih/errors.hh"
#include "nih/uri.hh"
#include "nih/strings.hh"
#include "nih/logging.hh"

namespace nih {

Uri::Uri(std::string uri)
    : _uri{uri},
      _is_valid {true},
      _code{UriErrorCode::kValid}
{
  std::vector<std::string> res;
  split(_uri, &res, [](char c) { return c == ':'; });
  if (res.size() == 0) {
    _is_valid = false;
    _code = UriErrorCode::kEmpty;
  } else {
    _scheme_str = res[0];
  }

  if (res.size() == 1) {
    _is_valid = false;
    _code = UriErrorCode::kHost;
  } else {
    auto host = res[1];
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
    : _scheme_str{scheme}, _host_str{host} {}

Uri& Uri::operator=(Uri const& that) {
  this->_uri = that._uri;
  this->_scheme_str = that._scheme_str;
  this->_host_str = that._host_str;
  this->_is_valid = that._is_valid;
  this->_code = that._code;
  return *this;
}

Uri& Uri::operator=(Uri&& that) {
  this->_uri = std::move(that._uri);
  this->_scheme_str = std::move(that._scheme_str);
  this->_host_str = std::move(that._host_str);
  this->_is_valid = that._is_valid;
  this->_code = that._code;
  return *this;
}

Uri& Uri::flush() {
  initialize();
  return *this;
}
}  // namespace nih