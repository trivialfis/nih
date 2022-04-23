#define NIH_LIKELY(x) __builtin_expect((x), 1)

#define NIH_UNLIKELY(x) __builtin_expect((x), 0)
