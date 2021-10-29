#include <stdio.h>
#include "threadPool.h"
#include <time.h>

#define MOD_FACTOR 100
ThreadPool* tp;


void hello (void* a)
{
   printf("hello\n");
}

void task1 (char *a){
    sleep(1);
    printf("%s\n",a);
}

void task2(void* a){
    sleep(1);
    srand(time(0));
    int limit = rand()%MOD_FACTOR;
    int i;
    printf("task 2 started. will loop %d times\n",limit);
    for(i=0;i<limit;i++){
        printf("%d) Random Task\n",i);
    }
}

void task3 (void* a){
    sleep(1);
    srand(time(0));
    int limit = rand()%MOD_FACTOR;
    int i,x,y;
    printf("task 3 started.");
    for (i=0;i<limit;i++){
        x = rand();
        y = rand();
        if (x>0 && y>0 && x+y<0){
            i--;
            continue;
        }
        printf("%d) %d + %d = %d\n",i+1,x,y,x+y);
    }
}


int test_thread_pool_sanity()
{
   int i,index,count=0;
   tp = tpCreate(20);
   void* funcs[] = {hello,task1,task2,task3};
   srand(time(0));
   for(i=0; i<150; ++i)
   {
       index = rand()%4;
       if (index != 1) {
           count+=tpInsertTask(tp, funcs[index % 4], NULL);
       }else {
           count+=tpInsertTask(tp, funcs[index % 4], "Some printable string");
       }
   }
   sleep(3);
   tpDestroy(tp,0);
    return count;

}


int main()
{
   int k = test_thread_pool_sanity();

   return k;
}
