#include <stdio.h>
#include <stdlib.h>
#include "osqueue.h"
#include "threadPool.h"
pthread_t th[5];


void hello (void* a)
{
   printf("hello\n");
}

void * all(void *arg){
   int i;
   
   ThreadPool* tp = tpCreate(5);
   
   for(i=0; i<5; ++i)
   {
      tpInsertTask(tp,hello,NULL);
   }
   
   tpDestroy(tp,0);
     return (NULL);
}
void test_thread_pool_sanity()
{
   int i, retcode;

  /* Create the threads */
  for(i = 0; i < 5; i++) {
  	retcode = pthread_create(&th[i], NULL, all, (void *)&i);
  	if (retcode != 0)
  		printf("Create thread failed with error %d\n", retcode);
  }
 
  /* Wait until all threads have finished */
  void* retVal[5];
  for(i = 0; i < 5; i++){
    pthread_join(th[i], &retVal[i]);
  }
}


int main()
{
   test_thread_pool_sanity();

   return 0;
}