#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

#define FALSE 0
#define TRUE 1

#define MAX 10000000
int arr_ind = 0;

int flag[2] = { FALSE, FALSE };
int turn;

int data[MAX] = { 0 };

void* thread_1(void*);
void* thread_2(void*);

int main()
{

    uint64_t sum = 0;
    pthread_t tid_1, tid_2;
    //-----------------BEGIN-------------------------
    pthread_create(&tid_1, NULL, thread_1, NULL);
    pthread_create(&tid_2, NULL, thread_2, NULL);

    void* status1;
    void* status2;

    pthread_join(tid_1, &status1);
    pthread_join(tid_2, &status2);
    //------------------END------------------------
    for (int i = 0; i < MAX; i++) {
        sum += data[i];
    }
    printf("SUM is %ld\n", sum);
    return 0;
}

void* thread_1(void* a)
{
    for (int i = 0; i < MAX;) {
        flag[0] = TRUE;
        turn = 1;
        while (flag[1] == TRUE && turn == 1)
            ;

        for (int j = 0; j < 100; j++) {
            data[arr_ind] = i;
            arr_ind++;
            i += 2;
        }

        flag[0] = FALSE;
    }
}

void* thread_2(void* a)
{

    for (int i = 0; i < MAX;) {
        flag[1] = TRUE;
        turn = 0;
        while (flag[0] == TRUE && turn == 0)
            ;

        for (int j = 0; j < 100; j++) {
            data[arr_ind] = i + 1;
            arr_ind++;
            i += 2;
        }
        flag[1] = FALSE;
    }
}