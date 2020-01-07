all:
	g++ -O0 tester.c -o tester -lpthread -lnuma -laio

aio:
	g++ -O0 aio_tester.cc -o aio_tester -lpthread