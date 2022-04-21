#include <vector>
#include <numeric>
#include <functional>
#include <algorithm>

namespace tungsten {
template <typename Idx, typename Container,
          typename V = typename Container::value_type,
          typename Comp = std::less<V>>
std::vector<Idx> ArgSort(Container const &array, Comp comp = std::less<V>{}) {
  std::vector<Idx> result(array.size());
  std::iota(result.begin(), result.end(), 0);
  auto op = [&array, comp](Idx const &l, Idx const &r) { return comp(array[l], array[r]); };
  std::stable_sort(result.begin(), result.end(), op);
  return result;
}
}  // namespace tungsten
