#pragma once
template <typename F> struct __Defer__ {
  F f;
  __Defer__(F f) : f(f) {}
  ~__Defer__() { f(); }
};

template <typename F> __Defer__<F> defer_func(F f) { return __Defer__<F>(f); }

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })

#define ito(N) for (u32 i = 0; i < N; ++i)
#define jto(N) for (u32 j = 0; j < N; ++j)
#define kto(N) for (u32 k = 0; k < N; ++k)

#define PERF_HIST_ADD(name, val)
#define PERF_ENTER(name)
#define PERF_EXIT(name)
#define OK_FALLTHROGH (void)0;