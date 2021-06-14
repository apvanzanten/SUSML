echo 'g++'
printf '#trans\ttime avg (msec)\n'
seq 5 5 25 | xargs -I {} sh -c "printf '{}\\t' && perf stat -x ';' -r 10 g++ ../tst/tuplebased.compiletest.cpp -I../ -O3 -std=c++17 -DNUM_TRANSITIONS={} -DNUM_TRIGGERS=100000 -o /dev/null 2>&1 >/dev/null | grep 'task-clock' | sed 's/;.*//'"

echo 'clang++'
printf '#trans\ttime avg (msec)\n'
seq 5 5 25 | xargs -I {} sh -c "printf '{}\\t' && perf stat -x ';' -r 10 clang++ ../tst/tuplebased.compiletest.cpp -I../ -O3 -std=c++17 -DNUM_TRANSITIONS={} -DNUM_TRIGGERS=100000 -o /dev/null 2>&1 >/dev/null | grep 'task-clock' | sed 's/;.*//'"
