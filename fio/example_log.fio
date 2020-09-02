[global]
thread
direct=1
ioengine=libaio 
group_reporting

# print log
log_avg_msec=1000
write_bw_log=write_job
write_iops_log=write_job
write_lat_log=write_job

# 记录IO -> 往文件的第[i]行，写了j大小的数据
# write_iolog=[]
# read_iolog=[]

[write_job]
size=10G
numjobs=8
bs=4K
rw=write
runtime=30
time_based