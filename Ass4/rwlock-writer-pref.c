#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define FILENAME "shared-filee.txt"

typedef struct __rwlock_t
{
    sem_t lock;          // binary semaphore for accessing counters
    sem_t writelock;     // exclusive access for writers
    sem_t readlock;      // blocks readers if a writer is waiting
    int readers;         // # of active readers
    int waiting_writers; // # of waiting writers
} rwlock_t;

void rwlock_init(rwlock_t *rw)
{
    rw->readers = 0;
    rw->waiting_writers = 0;
    sem_init(&rw->lock, 0, 1);
    sem_init(&rw->writelock, 0, 1);
    sem_init(&rw->readlock, 0, 1); // initialize as available for readers
}

void rwlock_acquire_readlock(rwlock_t *rw)
{
    sem_wait(&rw->readlock); // Block if a writer is waiting

    sem_wait(&rw->lock);
    rw->readers++;
    if (rw->readers == 1) // first reader locks writelock
        sem_wait(&rw->writelock);
    sem_post(&rw->lock);

    sem_post(&rw->readlock); // Allow other readers to proceed
}

void rwlock_release_readlock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) // last reader releases writelock
        sem_post(&rw->writelock);
    sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->waiting_writers++;
    if (rw->waiting_writers == 1) // first waiting writer blocks readers
        sem_wait(&rw->readlock);
    sem_post(&rw->lock);

    sem_wait(&rw->writelock); // acquire writelock for exclusive access
}

void rwlock_release_writelock(rwlock_t *rw)
{
    sem_post(&rw->writelock); // release the exclusive access

    sem_wait(&rw->lock);
    rw->waiting_writers--;
    if (rw->waiting_writers == 0) // last writer unblocks readers
        sem_post(&rw->readlock);
    sem_post(&rw->lock);
}

rwlock_t lock;

void *reader(void *arg)
{
    rwlock_acquire_readlock(&lock);
    printf("Reading, Number of readers present: [%d]\n", lock.readers);

    FILE *file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    char c;
    while ((c = fgetc(file)) != EOF)
        ; // simulate read
    fclose(file);

    rwlock_release_readlock(&lock);
    return NULL;
}

void *writer(void *arg)
{
    rwlock_acquire_writelock(&lock);
    printf("Writing, Number of readers present: [%d]\n", lock.readers);

    FILE *file = fopen(FILENAME, "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(file, "Hello World\n");
    fclose(file);

    rwlock_release_writelock(&lock);
    return NULL;
}

int main(int argc, char **argv)
{
    // Do not change the code below to spawn threads
    if (argc != 3)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t readers[n], writers[m];

    rwlock_init(&lock);
    // printf("Initialized rwlock\n");

    // Create reader and writer threads
    for (long i = 0; i < n; i++)
        pthread_create(&readers[i], NULL, reader, NULL);
    for (long i = 0; i < m; i++)
        pthread_create(&writers[i], NULL, writer, NULL);

    // Wait for all threads to complete
    for (int i = 0; i < n; i++)
        pthread_join(readers[i], NULL);
    for (int i = 0; i < m; i++)
        pthread_join(writers[i], NULL);

    return 0;
}
