#include "thread_add.h"

static __thread int count = 0;

void add()
{
    count++;
}

void print()
{
    printf("%d\n", count);
}