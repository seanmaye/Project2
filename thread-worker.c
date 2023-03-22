// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "thread-worker.h"
#define STACK_SIZE SIGSTKSZ
//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;

struct node *head = NULL;
struct node *running = NULL;
struct worker_mutex_t *keyHolder = NULL;
ucontext_t mcontext; 
ucontext_t scontext;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
int thread_count = 0;

void insert(struct TCB tcb) {
   //create a link
   struct node *new = (struct node*) malloc(sizeof(struct node));
	
   new->tcb = tcb;
	
   //point it to old first node
   new->next = head;
	
   //point first to new first node
   head = new;
}

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread
       // - allocate space of stack for this thread to run
       // after everything is set, push this thread into run queue and 
       // - make it ready for the execution.
	   //create scheduler context, main context and thread context 
       // YOUR CODE HERE
	 ucontext_t cctx;	 
	   void *tstack=malloc(STACK_SIZE);
	   struct TCB thread1 = {.tid =thread, .state= READY, .context = cctx, .tstack = tstack, .priority = 1};
	   thread_count ++;
	    
	
	if (tstack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}

	if(head == NULL){
		mcontext = cctx;
	}
      
		cctx.uc_link=NULL;
		cctx.uc_stack.ss_sp=tstack;
		cctx.uc_stack.ss_size=STACK_SIZE;
		cctx.uc_stack.ss_flags=0;
		makecontext(&cctx,&function, 1, arg);
		insert(thread1);
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
//find best fit for thread to pass context to 
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context
	// YOUR CODE HERE


	// for(struct node *curr = head; curr!=NULL; curr = curr->next){
	// 	if(curr->tcb.state == RUNNING){
	// 		curr->tcb.state = READY;
	// 		swapcontext(&curr->tcb.context, &scontext);
	// 		break;  
	// 	}
	// }
	running->tcb.state=READY;
	swapcontext(&running->tcb.context, &scontext);
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE
	if(value_ptr!=NULL){
		
	}
	struct node *current = head;
	current->tcb.state = TERMINATED;
	worker_yield();
	current->next = head;
	free(current->tcb.tstack);
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	struct node *target;
	for(struct node *curr = head; curr!=NULL; curr = curr->next){
		if(curr->thread == thread){
			target= curr;
			break;  
		}
	}
	if(target->tcb.state!='READY' || target->tcb.state!='BLOCKED'){
		while(target->tcb.state=target->tcb.state!='RUNNING' || target->tcb.state!='BLOCKED'){
			sleep(10); 
			printf("zzz\n");//problem here this def needs to be changed 
		}}else{
			target->tcb.state= 'READY';
			//the thread that is calling is now READY
		}
		return 0;
	}
	

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	if(running==NULL){
		printf("no threads running");
	}else{	
		mutex->tcb = running->tcb;
		mutex->key = 0;
		printf("hi4");
	}
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
		if (keyHolder == NULL){
			keyHolder = mutex;
		}
		else{
			struct node *curr = head;
			for(curr; curr!=NULL; curr = curr->next){
				if(curr->tcb.tid==mutex->tcb.tid){
					curr->tcb.state='BLOCKED';
				}
			}
			swapcontext(&curr->tcb.context, &scontext);
		}
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.
	// YOUR CODE HERE
	if (keyHolder == NULL){
			return 0;
		}
		else{
			struct node *curr = head;
			for(curr; curr!=NULL; curr = curr->next){
				if(curr->tcb.tid==mutex->tcb.tid){
					curr->tcb.state='BLOCKED';
				}
			}
			keyHolder=NULL;
		}
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init
	free(mutex->tcb.tstack);
	mutex = NULL;
	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
	sched_mlfq();
#else 
	sched_psjf();
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	
	printf("jhi");
	
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)
	printf("hi2");
	// YOUR CODE HERE
	
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

void getRunning( struct node* head){
	for(struct node *curr = head; curr!=NULL; curr = curr->next){
		if(curr->tcb.state == RUNNING){
			running = curr;
			break;  
		}
	}
}

// YOUR CODE HERE

