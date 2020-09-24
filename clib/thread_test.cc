#include "thread_add.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void* do_thread(void* v)
{
    int c = *((int*)v);
    for (int i = 0; i < c; i++) {
        add();
    }
    print();
    return NULL;
}

int main(int argc, char** argv)
{
    int count[4] = { 10, 20, 30, 40 };
    pthread_t threads[4];

    for (int i = 0; i < 4; i++) {
        pthread_create(threads + i, NULL, do_thread, (void*)(count + i));
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}