#ifndef _NIH_UNROLL_HH_
#define _NIH_UNROLL_HH_

#define ROUND_DOWN(N, step) ((N) & ~((step)-1))

#define LOOP_1(N, Unroll, Stmt)                                         \
  {                                                                     \
    size_t const __total_size = ROUND_DOWN((N), (Unroll));              \
    size_t __iter = 0;                                                  \
    size_t __c_iter = 0;                                                \
    for (; __iter < __total_size; __iter += (Unroll)) {                 \
      for (size_t __unroll_iter = 0; __unroll_iter < (Unroll); ++ __unroll_iter) { \
        __c_iter = __iter + __unroll_iter;                              \
        Stmt;                                                           \
      }                                                                 \
    }                                                                   \
    for (;__iter < (N); ++__iter) {                                     \
      __c_iter = __iter;                                                \
      Stmt;                                                             \
    }                                                                   \
  }

#endif