SPDK_LINK_FLAGS= -Wl,--whole-archive  -L../third-party/dpdk -lspdk_env_dpdk  -lspdk_env_dpdk_rpc \
    -L../third-party/spdk -ldpdk  \
    -lspdk_json -lspdk_jsonrpc -lspdk_log_rpc  -lspdk_app_rpc  -lspdk_rpc \
    -lspdk_bdev_rpc -lspdk_bdev_null -lspdk_bdev_malloc \
    -lspdk_bdev_nvme -lspdk_bdev_zone_block \
    -lspdk_bdev -lspdk_blob -lspdk_blob_bdev -lspdk_blobfs -lspdk_blobfs_bdev \
    -lspdk_event_bdev -lspdk_event_copy -lspdk_event_net -lspdk_event_vmd -lspdk_event \
    -lspdk_thread -lspdk_sock_posix -lspdk_sock -lspdk_notify \
    -lspdk_net \
    -lspdk_nvme \
    -lspdk_ftl \
    -lspdk_log -lspdk_trace -lspdk_util -lspdk_copy -lspdk_conf \
    -lspdk_vmd \
	-Wl,--no-whole-archive -lpthread -lrt -lnuma -ldl -luuid -lm -lisal

all:
	g++ -O0 bdev_example.cc -o bdev_example -I../include $(SPDK_LINK_FLAGS)
	g++ -O0 reactor_example.cc -o reactor_example -I../include $(SPDK_LINK_FLAGS)
	g++ -O0 blobstore_example.cc -o blobstore_example -I../include $(SPDK_LINK_FLAGS)

