#include <stdio.h>
#include <stdlib.h>
#include "osqueue.h"
#include "threadPool.h"
pthread_t th[5];


void hello (void* a)
{
   printf("hello\n");
}

void * insert(void *arg){
   int i;
   ThreadPool* tp=(ThreadPool*) arg;
   for(i=0; i<5; ++i)
   {
      tpInsertTask(tp,hello,NULL);
   }
     return (NULL);
}
void test_thread_pool_sanity()
{
    int i, retcode;
    ThreadPool* tp = tpCreate(5);

  
   for(i=0; i<5; ++i)
   {
      tpInsertTask(tp,hello,NULL);
   }
   

   /* Create the threads */
  for(i = 0; i < 5; i++) {
  	retcode = pthread_create(&th[i], NULL, insert, (void *)tp);
  	if (retcode != 0)
  		printf("Create thread failed with error %d\n", retcode);
  }
  
    /* Wait until all threads have finished */
      void* retVal[5];
      for(i = 0; i < 5; i++){
         pthread_join(th[i], &retVal[i]);
      }
      tpDestroy(tp,0);
  
   
   
}


int main()
{
   test_thread_pool_sanity();

   return 0;
}