# P3700
# warm_up
fio -group_reporting readwrite_warmup.fio

# seqwrite
echo 'P3700_readwrite...'
fio -group_reporting readwrite_test.fio > p3700_seqwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p3700/readwrite
mv *.svg p3700/readwrite
mv *.log p3700/readwrite

# randomread
echo 'P3700_randrws...'
fio -group_reporting randreadwrite_test.fio > p3700_randread.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p3700/randrw
mv *.svg p3700/randrw
mv *.log p3700/randrw

# P4800
# warm_up
fio -directory=/home/hanshukai/ssd2 -group_reporting readwrite_warmup.fio

# seqwrite
echo 'P4800_readwrite...'
fio -directory=/home/hanshukai/ssd2 -group_reporting readwrite_test.fio > p4800_seqwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p4800/seqwrite
mv *.svg p4800/seqwrite
mv *.log p4800/seqwrite

# randomwrite
echo 'P4800_readwrite...'
fio -directory=/home/hanshukai/ssd2 -group_reporting randreadwrite_test.fio > p4800_randwrite.log
./fio-fio-3.5/tools/fio_generate_plots mlog
mkdir -p p4800/randwrite
mv *.svg p4800/randwrite
mv *.log p4800/randwrite