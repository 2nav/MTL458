#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define FILENAME "shared-file.txt"

typedef struct __rwlock_t
{
    sem_t lock;      // binary semaphore (basic lock)
    sem_t writelock; // allow ONE writer or MANY readers
    int readers;     // #readers in critical section
} rwlock_t;

void rwlock_init(rwlock_t *rw)
{
    rw->readers = 0;
    sem_init(&rw->lock, 0, 1);
    sem_init(&rw->writelock, 0, 1);
}

void rwlock_acquire_readlock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->readers++;
    // printf("Readers present:[%d]\n", rw->readers);
    printf("Reading,Number-of-readers-present:[%d]\n", rw->readers);

    if (rw->readers == 1) // first reader gets writelock
        sem_wait(&rw->writelock);
    sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) // last reader lets it go
        sem_post(&rw->writelock);
    sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw)
{
    sem_wait(&rw->writelock);
}

void rwlock_release_writelock(rwlock_t *rw)
{
    sem_post(&rw->writelock);
}

rwlock_t lock;

void *reader(void *arg)
{
    // your code here
    rwlock_acquire_readlock(&lock);
    // printf("Reading,Number-of-readers-present:[%d]\n", lock.readers);

    // read shared-file.txt
    // https://stackoverflow.com/a/3463793
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    char c;
    while ((c = fgetc(file)) != EOF)
    {
        // printf("%c", c);
        ;
    }
    fclose(file);

    rwlock_release_readlock(&lock);
    return NULL;
}

void *writer(void *arg)
{
    // your code here
    rwlock_acquire_writelock(&lock);
    printf("Writing,Number-of-readers-present:[%d]\n", lock.readers);
    // append Hello World to shared-file.txt
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(file, "Hello world!\n");
    fclose(file);
    rwlock_release_writelock(&lock);
    return NULL;
}

int main(int argc, char **argv)
{

    // redirect stdout to output-reader-pref.txt
    freopen("output-reader-pref.txt", "w", stdout);

    // Do not change the code below to spawn threads
    if (argc != 3)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t readers[n], writers[m];

    rwlock_init(&lock);
    // printf("Initialized rwlock\n"); // debug, #TODO: remove

    // Create reader and writer threads
    for (int i = 0; i < n; i++)
        pthread_create(&readers[i], NULL, reader, NULL);
    for (int i = 0; i < m; i++)
        pthread_create(&writers[i], NULL, writer, NULL);

    // Wait for all threads to complete
    for (int i = 0; i < n; i++)
        pthread_join(readers[i], NULL);
    for (int i = 0; i < m; i++)
        pthread_join(writers[i], NULL);

    return 0;
}