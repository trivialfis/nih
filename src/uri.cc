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