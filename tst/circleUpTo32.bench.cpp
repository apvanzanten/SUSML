// This file is part of Still Untitled State Machine Library (SUSML).
//    Copyright (C) 2021 A.P. van Zanten
// SUSML is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// SUSML is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public License
// along with SUSML. If not, see <https://www.gnu.org/licenses/>.

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