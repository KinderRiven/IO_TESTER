mkdir perf_result

rm -rf 0.io
perf record ./tester 1 1 4096 2048
mv perf.data perf_result/sync_1
perf record ./tester 1 1 4096 2048
mv perf.data perf_result/sync_2
perf record ./tester 1 1 4096 2048
mv perf.data perf_result/sync_3

rm -rf 0.io
perf record ./tester 2 1 4096 2048
mv perf.data perf_result/sync_1
perf record ./tester 2 1 4096 2048
mv perf.data perf_result/sync_2
perf record ./tester 2 1 4096 2048
mv perf.data perf_result/sync_3

rm -rf 0.io
perf record ./tester 3 1 4096 2048
mv perf.data perf_result/sync_1
perf record ./tester 3 1 4096 2048
mv perf.data perf_result/sync_2
perf record ./tester 3 1 4096 2048
mv perf.data perf_result/sync_3