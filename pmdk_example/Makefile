PMDK_LINK_FLAGS= -L../lib/pmdk -lpmem -lpmemobj -lpmempool -lpthread -lrt

all:
	g++ -O0 pmdk_pmem_test.cc -o pmdk_pmem_test -I../include $(PMDK_LINK_FLAGS)