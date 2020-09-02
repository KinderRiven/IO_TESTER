DATASET="-name=file -size=50G -numjobs=4 -bs=16K"
LOG=" -log_avg_msec=5000 -write_bw_log=mlog -write_iops_log=mlog -write_lat_log=mlog"
IO=" -thread --direct=1 -ioengine=libaio"
TIME=" -runtime=3600 -time_based"

echo $DATASET
echo $IO
echo $TIME
echo $LOG

# P3700
# warm_up
fio -group_reporting  -rw=write $DATASET $IO $TIME

# seqwrite
echo 'P3700_seqwrite...'
fio -group_reporting  -rw=write $DATASET $IO $TIME $LOG > p3700_seqwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p3700/seqwrite
mv *.svg p3700/seqwrite
mv *.log p3700/seqwrite

# seqread
echo 'P3700_seqread...'
fio -group_reporting  -rw=read $DATASET $IO $TIME $LOG > p3700_seqread.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p3700/seqread
mv *.svg p3700/seqread
mv *.log p3700/seqread

# randomwrite
echo 'P3700_randwrite...'
fio -group_reporting  -rw=randwrite $DATASET $IO $TIME $LOG > p3700_randwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p3700/randwrite
mv *.svg p3700/randwrite
mv *.log p3700/randwrite

# randomread
echo 'P3700_randread...'
fio -group_reporting  -rw=randread $DATASET $IO $TIME $LOG > p3700_randread.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p3700/randread
mv *.svg p3700/randread
mv *.log p3700/randread

# P4800
# warm_up
fio -directory=/home/hanshukai/ssd2 -group_reporting  -rw=write $DATASET $IO $TIME

# seqwrite
echo 'P4800_seqwrite...'
fio -directory=/home/hanshukai/ssd2 -group_reporting  -rw=write $DATASET $IO $TIME $LOG > p4800_seqwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p4800/seqwrite
mv *.svg p4800/seqwrite
mv *.log p4800/seqwrite

# seqread
echo 'P4800_seqread...'
fio -directory=/home/hanshukai/ssd2 -group_reporting  -rw=read $DATASET $IO $TIME $LOG > p4800_seqread.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p4800/seqread
mv *.svg p4800/seqread
mv *.log p4800/seqread

# randomwrite
echo 'P4800_randwrite...'
fio -directory=/home/hanshukai/ssd2 -group_reporting  -rw=randwrite $DATASET $IO $TIME $LOG > p4800_randwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p4800/randwrite
mv *.svg p4800/randwrite
mv *.log p4800/randwrite

# randomread
echo 'P4800_randread...'
fio -directory=/home/hanshukai/ssd2 -group_reporting  -rw=randread $DATASET $IO $TIME $LOG > p4800_randread.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p4800/randread
mv *.svg p4800/randread
mv *.log p4800/randread