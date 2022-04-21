#define TS_LIKELY(x) __builtin_expect((x), 1)

#define TS_UNLIKELY(x) __builtin_expect((x), 0)
