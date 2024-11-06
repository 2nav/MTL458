// compile: gcc -o prod-cons prod-cons.c -lpthread
// run: ./prod-cons
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX 100

unsigned int buffer[MAX];
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

void print_buffer()
{
    printf("[");
    // print all elements which are present in the buffer, from use_ptr to fill_ptr-1, in a cyclic way
    int print_count = count;
    if (print_count == 0)
    {
        printf("]\n");
        return;
    }
    int i = use_ptr;
    int last = fill_ptr == 0 ? MAX - 1 : fill_ptr - 1;
    // skip 0
    if (buffer[last] == 0)
    {
        print_count--;
    }
    if (print_count == 0)
    {
        printf("]\n");
        return;
    }

    // printf("%d\n", print_count);
    while (--print_count > 0)
    {
        printf("%u,", buffer[i]);
        i = (i + 1) % MAX;
    }
    printf("%u", buffer[i]);

    printf("]\n");
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
        unsigned int buff;
        scanf("%u", &buff);
        put(buff);
        // terminate the producer if input is 0
        if (buff == 0)
        {
            pthread_cond_signal(&fill);
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

        unsigned int tmp = get();

        // Check for termination condition before printing
        if (tmp == 0)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        // Print buffer state after consuming the item
        printf("Consumed:[%u],Buffer-State:", tmp);
        print_buffer();

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    // read from input-pat1.txt
    freopen("input-part1.txt", "r", stdin);
    // redirect stdout to output-part1.txt
    freopen("output-part1.txt", "w", stdout);

    pthread_t p, c;
    // pthread_mutex_init(&mutex);
    // pthread_cond_init(&empty);
    // pthread_cond_init(&fill);
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);
    pthread_join(p, NULL);
    // printf("Producer thread joined\n");
    pthread_join(c, NULL);
    // printf("Consumer thread joined\n");
    return 0;
}