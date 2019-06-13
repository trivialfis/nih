#ifndef _REGISTRY_HH_
#define _REGISTRY_HH_

#include <functional>
#include <string>
#include <map>

namespace nih {

template <typename FType>
struct RegistryEntry {
 private:
  std::string _description;
  std::function<FType> _creator;

 public:
  RegistryEntry& describe(std::string description) {
    _description = description;
    return *this;
  }
  RegistryEntry& setCreator(std::function<FType> func) {
    _creator = func;
    return *this;
  }
  std::function<FType> getCreator() const {
    return _creator;
  }
};

template<typename EntryType>
class Registry {
 public:
  static std::map<std::string, EntryType>* getRegistry() {
    static std::map<std::string, EntryType> registry;
    return &registry;
  }
};


}      // namespace kapok

#endif  // _REGISTRY_HH_