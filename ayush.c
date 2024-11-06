// File: reader.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
// #include "zemaphore.h"  // Ensure this header provides sem_t, sem_init, sem_wait, sem_post
#include <semaphore.h>
// Structure for the read-write lock using semaphores with writer preference
typedef struct _rwlock_t
{
    sem_t lock;          // Semaphore to protect reader count
    sem_t writelock;     // Semaphore to control writers' access
    sem_t read_try;      // Semaphore to prevent new readers when writers are waiting
    int readers;         // Number of active readers
    int waiting_writers; // Number of writers waiting to write
} rwlock_t;

// Global read-write lock
rwlock_t rwlock;

// Semaphore to protect log file access
sem_t log_mutex;

// Function to initialize the read-write lock
void rwlock_init(rwlock_t *rw)
{
    rw->readers = 0;
    rw->waiting_writers = 0;
    if (sem_init(&rw->lock, 0, 1) != 0)
    {
        perror("Failed to initialize lock semaphore");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&rw->writelock, 0, 1) != 0)
    {
        perror("Failed to initialize writelock semaphore");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&rw->read_try, 0, 1) != 0)
    {
        perror("Failed to initialize read_try semaphore");
        exit(EXIT_FAILURE);
    }
}

// Function for readers to acquire the read lock (Writer Preference)
void rwlock_acquire_readlock(rwlock_t *rw)
{
    sem_wait(&rw->read_try); // Ensure no new readers enter if writers are waiting
    sem_wait(&rw->lock);
    rw->readers++;
    if (rw->readers == 1)
    {
        // First reader locks the writelock to block writers
        sem_wait(&rw->writelock);
    }
    sem_post(&rw->lock);
    sem_post(&rw->read_try); // Allow other readers if no writer is waiting
}

// Function for readers to release the read lock
void rwlock_release_readlock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->readers--;
    if (rw->readers == 0)
    {
        // Last reader releases the writelock to allow writers
        sem_post(&rw->writelock);
    }
    sem_post(&rw->lock);
}

// Function for writers to acquire the write lock
void rwlock_acquire_writelock(rwlock_t *rw)
{
    sem_wait(&rw->lock);
    rw->waiting_writers++;
    if (rw->waiting_writers == 1)
    {
        sem_wait(&rw->read_try); // First waiting writer blocks new readers
    }
    sem_post(&rw->lock);

    // Acquire writelock to gain exclusive access
    sem_wait(&rw->writelock);
}

// Function for writers to release the write lock
void rwlock_release_writelock(rwlock_t *rw)
{
    sem_post(&rw->writelock);

    sem_wait(&rw->lock);
    rw->waiting_writers--;
    if (rw->waiting_writers == 0)
    {
        // Last writer releases read_try, allowing readers to proceed
        sem_post(&rw->read_try);
    }
    sem_post(&rw->lock);
}

// Reader thread function
void *reader(void *arg)
{
    rwlock_acquire_readlock(&rwlock);

    // Critical Section (Reading)
    FILE *shared_file = fopen("shared-file.txt", "r");
    if (shared_file == NULL)
    {
        perror("Failed to open shared-file.txt");
        rwlock_release_readlock(&rwlock);
        pthread_exit(NULL);
    }

    // Read the file byte by byte
    int byte;
    while ((byte = fgetc(shared_file)) != EOF)
    {
        // Optional: Print each byte to stdout
    }
    fclose(shared_file);

    // Log reader activity
    sem_wait(&log_mutex);
    FILE *output_file = fopen("output-writer-pref.txt", "a");
    if (output_file == NULL)
    {
        perror("Failed to open output-writer-pref.txt");
        sem_post(&log_mutex);
        rwlock_release_readlock(&rwlock);
        pthread_exit(NULL);
    }
    fprintf(output_file, "Reading,Number-of-readers-present:[%d]\n", rwlock.readers);
    fclose(output_file);
    sem_post(&log_mutex);

    rwlock_release_readlock(&rwlock);

    pthread_exit(NULL);
}

// Writer thread function
void *writer(void *arg)
{
    rwlock_acquire_writelock(&rwlock);

    // Critical Section (Writing)
    FILE *shared_file = fopen("shared-file.txt", "a");
    if (shared_file == NULL)
    {
        perror("Failed to open shared-file.txt");
        rwlock_release_writelock(&rwlock);
        pthread_exit(NULL);
    }
    fprintf(shared_file, "Hello World\n");
    fclose(shared_file);

    sem_wait(&log_mutex);
    FILE *output_file = fopen("output-writer-pref.txt", "a");
    if (output_file == NULL)
    {
        perror("Failed to open output-writer-pref.txt");
        sem_post(&log_mutex);
        rwlock_release_writelock(&rwlock);
        pthread_exit(NULL);
    }
    fprintf(output_file, "Writing,Number-of-readers-present:[0]\n");
    fclose(output_file);
    sem_post(&log_mutex);

    rwlock_release_writelock(&rwlock);

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t readers[n], writers[m];

    rwlock_init(&rwlock);

    if (sem_init(&log_mutex, 0, 1) != 0)
    {
        perror("Failed to initialize log_mutex semaphore");
        return EXIT_FAILURE;
    }

    FILE *output_file = fopen("output-writer-pref.txt", "w");
    if (output_file != NULL)
    {
        fclose(output_file);
    }
    else
    {
        perror("Failed to create/clear output-writer-pref.txt");
    }

    for (int i = 0; i < n; i++)
        pthread_create(&readers[i], NULL, reader, NULL);
    for (int i = 0; i < m; i++)
        pthread_create(&writers[i], NULL, writer, NULL);

    for (int i = 0; i < n; i++)
        pthread_join(readers[i], NULL);
    for (int i = 0; i < m; i++)
        pthread_join(writers[i], NULL);

    return 0;
}