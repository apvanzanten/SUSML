#include <benchmark/benchmark.h>

#include "circleBench.util.hpp"

BENCH_CIRCLE(2);
BENCH_CIRCLE(4);
BENCH_CIRCLE(5);
BENCH_CIRCLE(6);
BENCH_CIRCLE(7);
BENCH_CIRCLE(8);
BENCH_CIRCLE(16);

BENCHMARK_MAIN();