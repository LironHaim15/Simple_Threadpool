// Liron Haim 206234635
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <pthread.h>
#include "sys/types.h"

typedef struct thread_pool
{
 //The field x is here because a struct without fields
 //doesn't compile. Remove it once you add fields of your own
 int size;
 int isBeingDestroyed;
 int destructorType; // 0 for rushed, 1 for patiant
 OSQueue *jobsQueue;
 pthread_t *threadsArray;
 pthread_mutex_t queueLock;
 pthread_mutex_t waitLock;
 pthread_mutex_t flagsCheckLock;
 pthread_cond_t condition;

} ThreadPool;

typedef struct job
{
    void (*computeFunc)(void *);
    void *param;
} Job;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
