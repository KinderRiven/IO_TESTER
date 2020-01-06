#include <stdio.h>
#include <fcntl.h>

// read/write
void io_read_write(int thread_id, size_t block_size, size_t total_size)
{
    int fd = open("1.io", O_RDWR, O_CREAT | O_DIRECT);
}

// direct_io
void io_direct_access(int thread_id, size_t block_size, size_t total_size)
{
}

// mmap
void io_mmap(int thread_id, size_t block_size, size_t total_size)
{
}

// async (libaio)
void io_libaio(int thread_id, size_t block_size, size_t total_size)
{
}

int main(int argc, char** argv)
{
    return 0;
}