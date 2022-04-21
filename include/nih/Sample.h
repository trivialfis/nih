#ifndef TS_SUPPORT_SAMPLE_H_
#define TS_SUPPORT_SAMPLE_H_

#include <vector>
#include <random>
#include <cassert>
#include <algorithm>
#include <numeric>

#include "Context.h"
#include "Logging.h"
#include "Omp.h"

namespace tungsten {

void RandomeCSRIndptr(Context &ctx, size_t n_samples, size_t n_features, float sparsity,
                      uint64_t *indptr_out) {
  auto density = 1 - sparsity;
  std::vector<std::normal_distribution<double>> t_dist{static_cast<size_t>(ctx.nThreads())};
  for (auto &dist : t_dist) {
    auto mean = static_cast<double>(n_features) * density;
    dist = std::normal_distribution<double>{mean};
  }
  std::vector<std::default_random_engine> t_randn(ctx.nThreads(), ctx.rng());
  for (size_t i = 0; i < t_randn.size(); ++i) {
    t_randn[i].seed(i + ctx.seed());
  }

  auto indptr = indptr_out;

  parallelFor<Schedule::kAuto>(n_samples, ctx.nThreads(), [&](size_t i) {
    auto &dist = t_dist[omp_get_thread_num()];
    auto v = dist(t_randn[omp_get_thread_num()]);
    v = std::max(0.0, v);
    v = std::min(v, static_cast<double>(n_features));
    auto ind = static_cast<size_t>(v);
    indptr[i + 1] = ind;
  });
  std::partial_sum(indptr, indptr + n_samples + 1, indptr);
  TS_ASSERT_EQ(indptr[0], 0);
}

template <typename T, typename Dist>
void RandomCSR(Context &ctx, size_t n_samples, size_t n_features, float sparsity, Dist distribution,
               uint64_t const *indptr, float *values_out, int32_t *indices_out) {
  std::vector<std::default_random_engine> t_randn(ctx.nThreads(), ctx.rng());
  for (size_t i = 0; i < t_randn.size(); ++i) {
    t_randn[i].seed(i + ctx.seed());
  }
  std::vector<Dist> t_dists{static_cast<size_t>(ctx.nThreads())};
  for (auto &dist : t_dists) {
    dist = distribution;
  }

  auto values = values_out;
  auto indices = indices_out;

  std::vector<int32_t> features(n_features);
  std::iota(features.begin(), features.end(), 0);

  parallelFor<Schedule::kAuto>(n_samples, ctx.nThreads(), [&](size_t i) {
    auto &t_rng = t_randn[omp_get_thread_num()];
    auto &t_dist = t_dists[omp_get_thread_num()];

    auto n_cols = indptr[i + 1] - indptr[i];
    assert(n_cols < static_cast<size_t>(std::numeric_limits<int32_t>::max()) && "Row is too long.");
    std::sample(features.cbegin(), features.cend(), indices + indptr[i], n_cols, t_rng);
    for (size_t j = indptr[i]; j < indptr[i + 1]; ++j) {
      values[j] = distribution(t_rng);
    }
  });
}
}  // namespace tungsten

#endif  // TS_SUPPORT_SAMPLE_H_
