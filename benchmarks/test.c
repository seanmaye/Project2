#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"
#include "../thread-worker.c"

// Let us create a global variable to change it in threads
int g = 0;

// The function to be executed by all threads
void *myThreadFun(void *vargp)
{
	printf("\nim a thread in a function \n");
}

int main()
{
	int i;
	pthread_t tid;

	// Let us create three threads
	for (i = 0; i < 1; i++){
		pthread_create(&tid, NULL, &myThreadFun, (void *)&tid);
        printf("created a thread in main! \n");
        
    }
	return 0;
}
