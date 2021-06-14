#include <benchmark/benchmark.h>

#include "circleBench.util.hpp"

using util::HasGuards;

BENCH_CIRCLE(64, HasGuards::no);
BENCH_CIRCLE(64, HasGuards::yes);

BENCHMARK_MAIN();