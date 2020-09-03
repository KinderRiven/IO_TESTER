all:
	g++ -O0 posix.cc -o posix -lpthread -lnuma -lnuma -laio
	g++ -O0 libaio.cc -o libaio -lpthread -lnuma -lnuma -laio