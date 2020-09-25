#include "utils.h"
#include <pthread.h>

struct task_queue_t {
public:
    volatile size_t task_idx; // Number of requests enqueued or in the process of being enqueued
    volatile size_t sending_tail; // Number of requests fully enqueued
    volatile size_t handing_tail; // Number of requests fully submitted and processed on disk
    size_t num_max_tasks; // Maximum number of enqueued requests
};

struct task_t {
public:
    char* key;
    char* value;
};

task_queue_t g_queue = {
    .task_idx = 0,
    .sending_tail = 0,
    .handing_tail = 0,
    .num_max_tasks = 128
};

task_t g_tasks[128];

#define NOP10() asm("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;")

size_t get_buffer_idx()
{
    size_t idx = __sync_fetch_and_add(&g_queue.task_idx, 1);
    return (idx % g_queue.num_max_tasks);
}

size_t enqueue_task(size_t idx)
{
    while (1) {
        if (g_queue.sending_tail % g_queue.num_max_tasks != idx) {
            NOP10();
        } else {
            break;
        }
    }
    return __sync_fetch_and_add(&g_queue.sending_tail, 1);
}

void do_batch_submit()
{
    size_t handing_tail = g_queue.handing_tail;
    size_t sending_tail = g_queue.sending_tail;
    size_t pending = sending_tail - handing_tail;

    for (size_t i = 0; i < pending; i++) {
        g_queue.handing_tail++;
        // __sync_fetch_and_add(&g_queue.handing_tail, 1);
    }
    printf("[do_batch_submit:%zu][%zu][%zu-%zu][%zu|%zu]\n",
        pending, pending, handing_tail, sending_tail,
        g_queue.sending_tail, g_queue.handing_tail);
}

void* do_thread(void* v)
{
    int id = *(int*)v;
    for (int i = 0; i < 100; i++) {
        size_t idx = get_buffer_idx();
        /* enqueue */
        size_t sending_tail = enqueue_task(idx);
        while (1) {
            wait_for(100000);
            /* only header thread can sumbit task */
            if (sending_tail < g_queue.handing_tail) {
                break;
            }
            if (sending_tail == g_queue.handing_tail) {
                do_batch_submit();
                break;
            }
        }
    }
    printf("Task%02d Finished!\n", id);
    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t threads[32];
    int pid[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    for (int i = 0; i < 4; i++) {
        pthread_create(threads + i, NULL, do_thread, (void*)(pid + i));
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}