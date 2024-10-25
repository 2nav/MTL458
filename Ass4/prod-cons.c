// include thread, locks, condition variable in linux
// compile: gcc -o prod-cons prod-cons.c -lpthread
// run: ./prod-cons
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX 100

int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;

void put(int value)
{
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % MAX;
    count++;
}

int get()
{
    int tmp = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % MAX;
    count--;
    return tmp;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    int i;
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (count == MAX)
            pthread_cond_wait(&empty, &mutex);
        int buff;
        scanf("%d", &buff);
        put(buff);
        // terminate the producer if input is 0
        if (buff == 0)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (count == 0)
            pthread_cond_wait(&fill, &mutex);
        int tmp = get();
        // terminate the consumer if input is 0
        if (tmp == 0)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
        printf("Consumed:[%d],Buffer_State:[]\n", tmp);
    }
}

int main(int argc, char *argv[])
{
    // read from input-pat1.txt
    freopen("input-part1.txt", "r", stdin);

    pthread_t p, c;
    // pthread_mutex_init(&mutex);
    // pthread_cond_init(&empty);
    // pthread_cond_init(&fill);
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    return 0;
}