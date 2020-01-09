RESULT_PATH=dd_result
BLOCK_SIZE=$((1*4096))
TOTAL_SIZE=$((4*1024))

mkdir $RESULT_PATH
for ((i=1; i<=3; i++))
do
rm -rf 0.io
dd if=/dev/zero of=0.io bs=$TOTAL_SIZE count=$((1024*1024))
for ((j=1; j<=10; j++))
do
echo "running $i_$j..."
# perf record ./tester $i 1 $BLOCK_SIZE $TOTAL_SIZE
./tester $i 1 $BLOCK_SIZE $TOTAL_SIZE
# mv perf.data $RESULT_PATH/$i_$j
sync && echo 3 > /proc/sys/vm/drop_caches
done
done
