// Liron Haim 206234635
#include "threadPool.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define IMPATIANT 0

void destroyAll(ThreadPool *threadPool)
{
    osDestroyQueue(threadPool->jobsQueue);
    free(threadPool->threadsArray);
    pthread_mutex_destroy(&threadPool->queueLock);
    pthread_mutex_destroy(&threadPool->waitLock);
    pthread_cond_destroy(&threadPool->condition);
    free(threadPool);
}

// function for handling errors. print the error, destroy variables and free allocaded memory. finally exit with value of -1.
void errorInFuncCall(char *msg, ThreadPool *threadPool)
{
    perror(msg);
    destroyAll(threadPool);
    exit(-1);
}

// join all the threads and wait for them to finish. on error return -1
int joinThreads(ThreadPool *threadPool)
{
    int i = 0;
    for (; i < threadPool->size; i++)
    {
        if (pthread_join(threadPool->threadsArray[i], NULL) != 0)
        {
            return -1;
        }
    }
    return 0;
}

// the main function of each thread. it is an endless loop that constantly pull jobs from the queue when the thread
// is awake. if no jobs were found then the thread is waiting until signaled. The loop ends when destruction is called.


void *runThread(void *tp)
{

    ThreadPool *threadPool = (ThreadPool *)tp;
    // main thread loop, runs until threadpool is destroyed or destroyed in patiant mode.
    while (threadPool->isBeingDestroyed != TRUE ||
           (threadPool->destructorType != IMPATIANT && !osIsQueueEmpty(threadPool->jobsQueue)))
    {

        Job *job = NULL;

        // pop the job from the head of the queue
        // mutex lock before modifying the queue
        if (pthread_mutex_lock(&threadPool->queueLock) != 0)
        {
            perror("Error in mutex lock");
            return NULL;
        }
        if (!osIsQueueEmpty(threadPool->jobsQueue))
        {
            job = (Job *)osDequeue(threadPool->jobsQueue);
        }
        // unlock mutex
        if (pthread_mutex_unlock(&threadPool->queueLock) != 0)
        {
            perror("Error in mutex unlock");
            return NULL;
        }

        // run the job with its parameters
        if (job != NULL)
        {
            job->computeFunc(job->param);
            free(job);
        }

        if (pthread_mutex_lock(&threadPool->waitLock) != 0)
        {
            perror("Error in mutex lock");
            return NULL;
        }

        // check if the threadpool is called for destruction and it is impatiant, or if job queue is empty
        while ((osIsQueueEmpty(threadPool->jobsQueue) && threadPool->isBeingDestroyed != TRUE)) // || osIsQueueEmpty(threadPool->jobsQueue))
        {
            // if so, then put thread to sleep until signal.
            if (pthread_cond_wait(&threadPool->condition, &threadPool->waitLock) != 0)
            {
                perror("Error in thread wait");
                return NULL;
            }
        }
        if (pthread_mutex_unlock(&threadPool->waitLock) != 0)
        {
            perror("Error in mutex unlock");
            return NULL;
        }
    }
    return NULL;
}

// create threads and save their id in the array in threadpool. on error return -1
int createThreads(ThreadPool *threadPool)
{
    int i = 0;
    for (; i < threadPool->size; i++)
    {
        if (pthread_create(threadPool->threadsArray + i, NULL, runThread, (void *)threadPool) != 0)
            return -1;
    }
    return 0;
}

// create a ThreadPool object
ThreadPool *tpCreate(int numOfThreads)
{

    // parameter check
    if (numOfThreads <= 0)
    {
        perror("Error in number of threads, invalid number");
        exit(-1);
    }

    //on any error in creation - destroy the threadpool and return NULL
    ThreadPool *threadPool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (threadPool == NULL)
    {
        perror("Error in allocating memory for threadpool");
        exit(-1);
    }

    // allocate memory for the threads array
    threadPool->threadsArray = (pthread_t *)malloc(sizeof(pthread_t) * numOfThreads);
    if (threadPool->threadsArray == NULL)
    {
        free(threadPool);
        perror("Error in allocating memory for threads");
        exit(-1);
    }

    // initate threadpool fields.
    threadPool->size = numOfThreads;
    threadPool->jobsQueue = osCreateQueue();
    threadPool->isBeingDestroyed = FALSE;
    threadPool->destructorType = 1;

    // initiate mutex locks and thread condition variables.
    if (pthread_mutex_init(&threadPool->queueLock, NULL) != 0)
    {
        perror("Error in mutex lock intilization");
        osDestroyQueue(threadPool->jobsQueue);
        free(threadPool->threadsArray);
        free(threadPool);
        exit(-1);
    }
    if (pthread_cond_init(&threadPool->condition, NULL) != 0)
    {
        errorInFuncCall("Error in thread condition intilization", threadPool);
        pthread_mutex_destroy(&threadPool->queueLock);
        osDestroyQueue(threadPool->jobsQueue);
        free(threadPool->threadsArray);
        free(threadPool);
        exit(-1);
    }

    if (pthread_mutex_init(&threadPool->waitLock, NULL) != 0)
    {
        perror("Error in mutex lock intilization");
        osDestroyQueue(threadPool->jobsQueue);
        pthread_mutex_destroy(&threadPool->queueLock);
        pthread_cond_destroy(&threadPool->condition);
        free(threadPool->threadsArray);
        free(threadPool);
        exit(-1);
    }

    // create 'numOfThreads' amount of threads and save their ids in the array in the threadpool.
    if (createThreads(threadPool) != 0)
    {
        errorInFuncCall("Error in creating threads", threadPool);
    }
    return threadPool;
}

// destroy the Threadpool. two modes are posssible:
// 1. PATIANT MODE when the threadpool finishes all the job in the queue, cannot add more jobs.
// 2. IMPATIANT MODE when only the jobs the threads are already holding will finish and ignore the rest of the job queue, cannot add more jobs.
void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks)
{
    // check parameters and if already called for destruction
    if (threadPool == NULL || threadPool->isBeingDestroyed == TRUE)
    {
        return;
    }

    // lock the queue to prevent other threads to pull jobs from.
    if (pthread_mutex_lock(&threadPool->queueLock) != 0)
    {
        errorInFuncCall("Error in mutex lock", threadPool);
    }

    // change flag of destruction to TRUE.
    threadPool->isBeingDestroyed = TRUE;
    threadPool->destructorType = shouldWaitForTasks;
    // unlock mutex
    if (pthread_mutex_unlock(&threadPool->queueLock) != 0)
    {
        errorInFuncCall("Error in mutex unlock", threadPool);
    }

    // lock mutex for signals
    if (pthread_mutex_lock(&threadPool->waitLock) != 0)
    {
        errorInFuncCall("Error in mutex lock", threadPool);
    }

    // wake up all the threads.
    if (pthread_cond_broadcast(&threadPool->condition) != 0)
    {
        errorInFuncCall("Error in broadcast", threadPool);
    }
    // unlock mutex for signals
    if (pthread_mutex_unlock(&threadPool->waitLock) != 0)
    {
        errorInFuncCall("Error in mutex unlock", threadPool);
    }

    // join threads and wait for them to finish.
    if (joinThreads(threadPool) != 0)
        errorInFuncCall("Error in joining threads", threadPool);

    // clear the job queue if it is an impatiant destruction call.
    if (threadPool->destructorType == IMPATIANT)
    {
        Job *job;
        while (!osIsQueueEmpty(threadPool->jobsQueue))
        {
            job = (Job *)osDequeue(threadPool->jobsQueue);
            free(job);
        }
    }

    // destroy jobs queue, mutex lock, thread condition and free allocated memory for the threadpool.
    destroyAll(threadPool);
}

// insert a job to the jobs queue.
int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param)
{

    // valid parameters check
    if (threadPool == NULL)
    {
        return -1;
    }
    if (computeFunc == NULL)
    {
        return -1;
    }
    // check if the threadPool was called for destruction
    if (threadPool->isBeingDestroyed == TRUE)
    {
        return -1;
    }

    // create new job
    Job *newJob = (Job *)malloc(sizeof(Job)); ////free
    if (computeFunc != NULL)
    {
        newJob->computeFunc = computeFunc;
        newJob->param = param;
    }

    // mutex lock before modifying the queue
    if (pthread_mutex_lock(&threadPool->queueLock) != 0)
    {
        errorInFuncCall("Error in mutex lock", threadPool);
    }

    // add new job to queue
    osEnqueue(threadPool->jobsQueue, newJob);

    // mutex unlock after modifying the queue
    if (pthread_mutex_unlock(&threadPool->queueLock) != 0)
    {
        errorInFuncCall("Error in mutex unlock", threadPool);
    }

    if (pthread_mutex_lock(&threadPool->waitLock) != 0)
    {
        errorInFuncCall("Error in mutex lock", threadPool);
    }
    // send a signal to wake up thread for the new job
    if (pthread_cond_signal(&threadPool->condition) != 0)
    {
        errorInFuncCall("Error in signal to threads", threadPool);
    }
    // mutex unlock after modifying the queue
    if (pthread_mutex_unlock(&threadPool->waitLock) != 0)
    {
        errorInFuncCall("Error in mutex unlock", threadPool);
    }

    // added successfully to queue
    return 0;
}
