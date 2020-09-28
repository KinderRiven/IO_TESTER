#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

int main()
{
    char* pool = new char[1024 * 1024];
    printf("%llu\n", (uint64_t)pool);
    for (int i = 0; i < 10; i++) {
        std::string* p = new (pool) std::string("hello");
        printf("%llu\n", (uint64_t)p);
    }
    return 0;
}