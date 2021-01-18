PMDK_LINK_FLAGS= -L../lib/pmdk -lpmem -lpmemobj -lpmempool -lpmemlog -lpthread -lrt

all: pmem pmemlog pmemobj

pmem:
	g++ -O0 pmdk_pmem_test.cc -o pmdk_pmem_test -I../include $(PMDK_LINK_FLAGS)

pmemlog:
	g++ -O0 pmdk_log_test.cc -o pmdk_log_test -I../include $(PMDK_LINK_FLAGS)

pmemobj:
	g++ -O0 pmdk_object_test.cc -o pmdk_object_test -I../include $(PMDK_LINK_FLAGS)