echo 'without asserts'
printf '#trans\ttime avg (msec)\n'
seq 5 5 25 | xargs -I {} sh -c "printf '{}\\t' && perf stat -x ';' -r 10 g++ ../tst/tuplebased.compiletest.cpp -I../ -O3 -DNUM_TRANSITIONS={} -DNUM_TRIGGERS=10000 -DUSE_ASSERTS=false -o /dev/null 2>&1 >/dev/null | grep 'task-clock' | sed 's/;.*//'"

echo 'with asserts'
printf '#trans\ttime avg (msec)\n'
seq 5 5 25 | xargs -I {} sh -c "printf '{}\\t' && perf stat -x ';' -r 10 g++ ../tst/tuplebased.compiletest.cpp -I../ -O3 -DNUM_TRANSITIONS={} -DNUM_TRIGGERS=10000 -DUSE_ASSERTS=true -o /dev/null 2>&1 >/dev/null | grep 'task-clock' | sed 's/;.*//'"
