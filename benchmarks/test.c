#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"
#include "../thread-worker.c"

// Let us create a global variable to change it in threads
int g = 0;
pthread_mutex_t lock;

// The function to be executed by all threads
void myThreadFun()
{	
	
	printf("\nim a thread in a function \n");
	pthread_exit(NULL);

}
void incramenter(){
	for (int i = 0; i < 26; i++){
		if(g==15) {
			printf("finshed a thread to test mutex");
			g++;
			pthread_exit(NULL);
			return;
			}
		if (g >= 25){
		printf("done\n");
		
	}
	else{
		printf("g is currently %i\n",g);
	g++;
	}
	}
	pthread_exit(NULL);

}

int main()
{
	int i;
	pthread_t tid;	
	int error;
	

	// Let us create three threads
		pthread_create(&tid, NULL, &incramenter, NULL);
        printf("created a thread in main!\n");
		

		//pthread_create(&tid, NULL, &incramenter, NULL);
        printf("created a thread in main!\n");
		

	
	return 0;
}
