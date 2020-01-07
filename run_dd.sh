RESULT_PATH=dd_result
mkdir $RESULT_PATH

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 1 1 4096 2048
mv perf.data $RESULT_PATH/sync_1
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 1 1 4096 2048
mv perf.data $RESULT_PATH/sync_2
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 1 1 4096 2048
mv perf.data $RESULT_PATH/sync_3
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 2 1 4096 2048
mv perf.data $RESULT_PATH/sync_1
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 2 1 4096 2048
mv perf.data $RESULT_PATH/sync_2
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 2 1 4096 2048
mv perf.data $RESULT_PATH/sync_3
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 3 1 4096 2048
mv perf.data $RESULT_PATH/sync_1
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 3 1 4096 2048
mv perf.data $RESULT_PATH/sync_2
sync && echo 3 > /proc/sys/vm/drop_caches

rm -rf 0.io
dd if=/dev/zero of=0.io bs=$((2*1024*1024)) count=$((1024))
perf record ./tester 3 1 4096 2048
mv perf.data $RESULT_PATH/sync_3
sync && echo 3 > /proc/sys/vm/drop_caches