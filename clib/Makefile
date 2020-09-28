all: thread task_queue
	echo 'thread + task'

thread:
	g++ thread_add.cc thread_test.cc -o thread_add -lpthread

task_queue:
	g++ task_queue_test.cc -o task_queue_test -lpthread

malloc:
	g++ placement_new.cc -o  placement_new -lpthread

clean:
	rm -rf task_queue_test thread_add