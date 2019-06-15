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

#include <nih/errors.hh>
#include <nih/uri.hh>
#include <nih/strings.hh>

namespace nih {

Uri::Uri(std::string uri, std::string flags)
    : _uri{uri},
      _flags{flags},
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
    _host_str = res[1];
  }
}

Uri& Uri::operator=(Uri const& that) {
  this->_uri = that._uri;
  this->_scheme_str = that._scheme_str;
  this->_host_str = that._host_str;
  this->_flags = that._flags;
  this->_is_valid = that._is_valid;
  this->_code = that._code;

  this->_scheme = that._scheme;
  return *this;
}

Uri& Uri::operator=(Uri&& that) {
  this->_uri = std::move(that._uri);
  this->_scheme_str = std::move(that._scheme_str);
  this->_host_str = std::move(that._host_str);
  this->_flags = std::move(that._flags);
  this->_is_valid = that._is_valid;
  this->_code = that._code;

  this->_scheme = std::move(that._scheme);
  return *this;
}

Uri& Uri::write(std::string const& input) {
  initialize();
  _scheme->write(input);
  return *this;
}
Uri& Uri::write(char* input, size_t size) {
  initialize();
  _scheme->write(input, size);
  return *this;
}

Uri& Uri::read(std::string* output, size_t size) {
  initialize();
  _scheme->read(output, size);
  return *this;
}
Uri& Uri::read(char* output, size_t size) {
  initialize();
  _scheme->read(output, size);
  return *this;
}

Uri& Uri::flush() {
  initialize();
  _scheme->flush();
  return *this;
}

UriScheme* UriScheme::create(std::string scheme, Uri const * const uri){
  auto registry = Registry<RegistryT>::getRegistry();
  auto iter = registry->find(scheme);
  if (iter == registry->cend()) {
    std::string msg {"Available Schemes:\n"};
    for (auto const& kv : *registry) {
      msg += "# " + kv.first + '\n';
    }
    throw NIHError(msg);
  }
  auto ptr = iter->second.getCreator()(*uri, uri->flags());
  return ptr;
}

UriScheme::RegistryT& UriScheme::registry(
    std::string name, std::function<UriScheme*(Uri const&, std::string flags)> creator) {
  auto registry = Registry<RegistryT>::getRegistry();
  if (registry->find(name) != registry->cend()) {
    throw NIHError("Uri scheme: " + name + " is already registered.");
  }
  (*registry)[name].setCreator(creator);
  return (*registry)[name];
}

}  // namespace nih