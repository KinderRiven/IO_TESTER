mkdir perf_result

# sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
perf record ./tester 1 1 4096 2048
mv perf.data perf_result/sync_1
sync && echo 3 > /proc/sys/vm/drop_caches
perf record ./tester 1 1 4096 2048
mv perf.data perf_result/sync_2
sync && echo 3 > /proc/sys/vm/drop_caches
perf record ./tester 1 1 4096 2048
mv perf.data perf_result/sync_3
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
perf record ./tester 2 1 4096 2048
mv perf.data perf_result/sync_1
sync && echo 3 > /proc/sys/vm/drop_caches
perf record ./tester 2 1 4096 2048
mv perf.data perf_result/sync_2
sync && echo 3 > /proc/sys/vm/drop_caches
perf record ./tester 2 1 4096 2048
mv perf.data perf_result/sync_3
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
perf record ./tester 3 1 4096 2048
mv perf.data perf_result/sync_1
sync && echo 3 > /proc/sys/vm/drop_caches
perf record ./tester 3 1 4096 2048
mv perf.data perf_result/sync_2
sync && echo 3 > /proc/sys/vm/drop_caches
perf record ./tester 3 1 4096 2048
mv perf.data perf_result/sync_3
sync && echo 3 > /proc/sys/vm/drop_caches