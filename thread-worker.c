// File:	thread-worker.c

// List all group member's name:
// username of iLab: sam710
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
ucontext_t t1context;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
int thread_count = 0;

void insert(struct TCB tcb) {
   // create a link
   struct node *new_node = (struct node*) malloc(sizeof(struct node));
   new_node->tcb = tcb;
   new_node->next = NULL;
   new_node->prev = NULL;
   

   // add the new node to the list
   if (head == NULL) {
      // the list is empty, so make this node the head
      head = new_node;
   } else {
      // traverse the list to find the last node
      struct node *last = head;
      while (last->next != NULL) {
         last = last->next;
      }
      // add the new node after the last node
      last->next = new_node;
      new_node->prev = last;
   }
}



/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE

	
	while(running->tcb.state==TERMINATED){
		running=running->next;
	}
	setcontext(&running->tcb.context);//each thread will call exit so maybe we dont need to do this?
	printf("running thread address : %p \n",&running);
	printf("this is a attempt at a firtst come first serve");
	printf("in psjf");
	
	
	
	//somehow gotta changed the queue i dont know where 
	
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)
	printf("in mlfq\n");
	running = (head);
	
	printf("running thread address : %p \n",&running->tcb.context);
	setcontext(&running->tcb.context);
	printf("swapping from scontext to t1");
	// swapcontext(&scontext,&running->tcb.context);//each thread will call exit so maybe we dont need to do this?
	printf("running thread address : %p \n",&running);
	printf("this is a attempt at a firtst come first serve and we are on thread %i\n",thread_count);	
	
	
	// YOUR CODE HERE
	
}

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
/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void*), void *arg) {
    // Create a TCB for the new thread
    TCB *new_tcb = (TCB*) malloc(sizeof(TCB));
    if (new_tcb == NULL) {
        perror("Failed to allocate memory for TCB");
        return -1;
    }
    
    // Allocate a new stack for the new thread
    new_tcb->tstack = (char*) malloc(STACK_SIZE);
    if (new_tcb->tstack == NULL) {
        perror("Failed to allocate memory for thread stack");
        free(new_tcb);
        return -1;
    }
    
    // Initialize the new TCB
    new_tcb->tid = thread;
    new_tcb->state = READY;
    new_tcb->priority = 0;
    new_tcb->lock = 0;
    new_tcb->contextSwitches = 0;
    new_tcb->retVal = NULL;
    
    // Initialize the context for the new thread
    getcontext(&new_tcb->context);
    new_tcb->context.uc_stack.ss_sp = new_tcb->tstack;
    new_tcb->context.uc_stack.ss_size = SIGSTKSZ;
    new_tcb->context.uc_link = NULL;
    makecontext(&new_tcb->context, (void(*)(void)) function, 1, arg);

	if(head == NULL){
		getcontext(&mcontext);
		scontext.uc_stack.ss_sp= malloc(STACK_SIZE);
		scontext.uc_stack.ss_size = STACK_SIZE;
		scontext.uc_link=NULL;
		getcontext(&scontext);
		makecontext(&scontext,(void(*)(void))&schedule,0);
		printf("Main Context at %p:\n", mcontext);
		printf("Scheduler Context at %p:\n", scontext);
		printf("tcbnew Context at %p:\n", new_tcb->context);
		printf("scheduler and main context are made \n");
	}

    // Add the new TCB to the end of the linked list of TCBs
    struct node *new_node = (struct node*) malloc(sizeof(struct node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for linked list node");
        free(new_tcb->tstack);
        free(new_tcb);
        return -1;
    }
    new_node->thread = *thread;
    new_node->tcb = *new_tcb;
    new_node->next = NULL;
    new_node->prev = NULL;
	
    
    if (head == NULL) {
        head = new_node;
    } else {
        struct node *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;
    }
	insert(*new_tcb);
    swapcontext(&mcontext,&new_tcb->context);
    return 0;
}


/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
//find best fit for thread to pass context to 
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context
	// YOUR CODE HERE
	printf("Scheduler Context at %p:\n", scontext);
	running=head;
	running->tcb.state=READY;
	tot_cntx_switches++;
	swapcontext(&running->tcb.context, &scontext);
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE
	printf("running thread address : %p \n",&running);
	running=head;
	if(running==NULL){
		printf("current doesnt exist, no thread is running \n");
		printf("Scheduler Context at %p:\n", scontext);
		setcontext(&mcontext);
	}
	if(value_ptr!=NULL){
		struct node *current = running;
		current->tcb.state = TERMINATED;
		free(current->tcb.tstack);
		head = current->next;
	}
	
	setcontext(&mcontext);
	
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	 // - wait for a specific thread to terminate
    // - de-allocate any dynamic memory created by the joining thread
  
    // YOUR CODE HERE
    struct node *current = head;
	printf("inside worker_join\n");
    while (current != NULL) {
        if (current->tcb.tid == thread) {
            if (current->tcb.state == TERMINATED) {
                // Deallocate any memory allocated by the terminated thread
                free(current->tcb.tstack);
                // Remove the node from the list
                if (current == head) {
                    head = current->next;
                } else {
                    struct node *prev = head;
                    while (prev->next != current) {
                        prev = prev->next;
                    }
                    prev->next = current->next;
                }
                free(current);
                return 0;
            } else {
                // Wait for the thread to terminate
                running->tcb.state = 'READY';
                keyHolder = current;
                swapcontext(&running->tcb.context, &scontext);
				printf("swapping from running to scheduler");
                break;
            }
        }
        current = current->next;
    }
    // The thread with the specified ID was not found
    return -1;
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
					curr->tcb.state=BLOCKED;
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
					curr->tcb.state=BLOCKED;
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

