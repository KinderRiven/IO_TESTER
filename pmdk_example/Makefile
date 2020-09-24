PMDK_LINK_FLAGS= -Wl,--whole-archive  -Llib/pmdk -lpmem -lpmemobj -lpmempool -lpmem2
	-Wl,--no-whole-archive -lpthread -lrt -lnuma -ldl -luuid -lm -lisal

all:
	g++ -O0 pmdk_pmem_test.cc -o pmdk_pmem_test -I../include $(PMDK_LINK_FLAGS)