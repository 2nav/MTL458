#include <pthread.h>
#include <semaphore.h>
// #include "zemaphore.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_FILENAME "output-writer-pref.txt"
#define SHARED_FILENAME "shared-file.txt"
#define WRITE_LOG_MESSAGE "Writing,Number-of-readers-present:[%d]\n"
#define READ_LOG_MESSAGE "Reading,Number-of-readers-present:[%d]\n"
#define WRITE_MESSAGE "Hello world!\n"
#define IO_ERROR_MESSAGE "Error opening file!\n"


void exit_error(const char *message)
{
    perror(message);
    exit(1);
}

typedef struct _rwlock_t
{
    sem_t lock;
    sem_t ok_to_read;
    // this is used as a condition variable for signaling
    sem_t resource;
    // resource: only one of reading (possibly multiple) or writing (single) occurs at a time
    sem_t log_lock;
    // for logging into output file so no more than one reader is trying to write at a time
    int readers;
    int writers_waiting;
    // used to implement writer preference later
} rwlock_t;

void rwlock_init(rwlock_t *rw)
{
    sem_init(&rw->lock, 0, 1);
    sem_init(&rw->ok_to_read, 0, 0);
    sem_init(&rw->resource, 0, 1);
    sem_init(&rw->log_lock, 0, 1);
    rw->readers = 0;
    rw->writers_waiting = 0;
}

void rwlock_acquire_readlock(rwlock_t *rw)
{

    sem_wait(&rw->lock);            // for reading writers_waiting variable
    while (rw->writers_waiting > 0) // not spinning. safe not to use if
    {
        sem_post(&rw->lock);       // release the lock and sleep
        sem_wait(&rw->ok_to_read); // sleep until signaled
        sem_wait(&rw->lock);       // reaquire lock is awake. now read writers_waiting again
    }
    if (rw->readers == 0) // the first reader
        sem_wait(&rw->resource);
    rw->readers++; // important. increment after acquired
    sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw)
{

    sem_wait(&rw->lock);
    if (rw->readers == 1)
    {
        sem_post(&rw->resource);
        /*
        this signals the thread sleeping in the while() in rwlock_acquire_readlock() to wake up
        otherwise deadlock. as reader threads my get trapped. this is safe as the while() loop is there in rwlock_acquire_readlock
        so even if of_to_read has value more than 1, it's handles in the while loop.
        */
        if (rw->writers_waiting == 0)
            sem_post(&rw->ok_to_read);
    }
    rw->readers--;
    sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->writers_waiting++;
    sem_post(&rw->lock);

    sem_wait(&rw->resource);
    // realease the lcik while waiting for resource. else deadlock

    sem_wait(&rw->lock);
    rw->writers_waiting--;
    sem_post(&rw->lock);
}

void rwlock_release_writelock(rwlock_t *rw)
{

    sem_post(&rw->resource); // must be before acquiring the lock below else it leads to deadlock
    sem_wait(&rw->lock);
    /*
    this signals the thread sleeping in the while() in rwlock_acquire_readlock() to wake up
    */
    if (rw->writers_waiting == 0)
        sem_post(&rw->ok_to_read);

    sem_post(&rw->lock);
}

rwlock_t rwlock;

void *reader(void *arg)
{
    rwlock_acquire_readlock(&rwlock);
    sem_wait(&rwlock.log_lock);
    FILE *logfile = fopen(OUTPUT_FILENAME, "a");
    if (logfile == NULL)
        exit_error(IO_ERROR_MESSAGE);
    fprintf(logfile, READ_LOG_MESSAGE, rwlock.readers);
    fclose(logfile);
    sem_post(&rwlock.log_lock);

    FILE *shared_file = fopen(SHARED_FILENAME, "r");
    if (shared_file)
    {
        volatile int ch; // to prevent compiler from optimizing the loop
        while ((ch = fgetc(shared_file)) != EOF)
        {
        }
        fclose(shared_file);
    }
    else
    {
        exit_error(IO_ERROR_MESSAGE);
    }

    rwlock_release_readlock(&rwlock);
    return NULL;
}

void *writer(void *arg)
{
    rwlock_acquire_writelock(&rwlock);
    sem_wait(&rwlock.log_lock);
    FILE *logfile = fopen(OUTPUT_FILENAME, "a");
    if (logfile == NULL)
        exit_error(IO_ERROR_MESSAGE);
    fprintf(logfile, WRITE_LOG_MESSAGE, rwlock.readers);
    fclose(logfile);
    sem_post(&rwlock.log_lock);
    FILE *shared_file = fopen(SHARED_FILENAME, "a");
    if (shared_file)
    {
        fprintf(shared_file, WRITE_MESSAGE);
        fclose(shared_file);
    }
    else
    {
        exit_error(IO_ERROR_MESSAGE);
    }

    rwlock_release_writelock(&rwlock);
    return NULL;
}

int main(int argc, char **argv)
{
    rwlock_init(&rwlock);
    if (argc != 3)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t readers[n], writers[m];

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
