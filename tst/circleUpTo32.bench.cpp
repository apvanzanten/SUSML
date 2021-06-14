#include <benchmark/benchmark.h>

#include "circleBench.util.hpp"

using util::HasGuards;

BENCH_CIRCLE(2, HasGuards::no);
BENCH_CIRCLE(4, HasGuards::no);
BENCH_CIRCLE(5, HasGuards::no);
BENCH_CIRCLE(6, HasGuards::no);
BENCH_CIRCLE(7, HasGuards::no);
BENCH_CIRCLE(8, HasGuards::no);
BENCH_CIRCLE(16, HasGuards::no);
BENCH_CIRCLE(20, HasGuards::no);
BENCH_CIRCLE(24, HasGuards::no);
BENCH_CIRCLE(28, HasGuards::no);
BENCH_CIRCLE(32, HasGuards::no);
BENCH_CIRCLE(2, HasGuards::yes);
BENCH_CIRCLE(4, HasGuards::yes);
BENCH_CIRCLE(5, HasGuards::yes);
BENCH_CIRCLE(6, HasGuards::yes);
BENCH_CIRCLE(7, HasGuards::yes);
BENCH_CIRCLE(8, HasGuards::yes);
BENCH_CIRCLE(16, HasGuards::yes);
BENCH_CIRCLE(20, HasGuards::yes);
BENCH_CIRCLE(24, HasGuards::yes);
BENCH_CIRCLE(28, HasGuards::yes);
BENCH_CIRCLE(32, HasGuards::yes);

BENCHMARK_MAIN();