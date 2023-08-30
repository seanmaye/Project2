// File:	thread-worker.c

// List all group member's name:Sean Maye , Andrew San
// username of iLab: sam710 ars400
// iLab Server: ilab4

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
struct itimerval timer;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
int thread_count = 0;

// Define an array of run queues
struct runqueue {
    struct node *head;
    int quantum;
} runqueues[NUM_LVLS] = {
    {NULL, 25},  
    {NULL, 50}, 
    {NULL, 75},
	{NULL, 100}  
};

void insert(struct TCB tcb) {
   // create a link
   struct node *new_node = (struct node*) malloc(sizeof(struct node));
   new_node->tcb = tcb;
 
    new_node->next = head;
    new_node->prev = NULL;
 
   
    if (head != NULL)
        head->prev = new_node;
 
    
    head = new_node;
}



/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
   // find the thread with the shortest expected processing time
   struct node *current = head;
   if (head == NULL) exit;

   struct node *shortest = head;
	for(current ; current->next != NULL; current = current->next){
		if (current->tcb.quantums < shortest->tcb.quantums) shortest = current;
	}
	setitimer(ITIMER_PROF, &timer, NULL);
	current->tcb.state = RUNNING;
	running=current;
	tot_cntx_switches++;
	swapcontext(&scontext,&current->tcb.context);
}



/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// YOUR CODE HERE
	// Run the highest priority queue with non-empty threads
    for (int i = 0; i < NUM_LVLS; i++) {
		for(struct node *curr = head; curr->next!=NULL;curr = curr->next){
			struct node *temp = head;
			struct node *last = head;
			head = head-> next;
			for(last; last!=NULL;last = last->next){

			}
			last->next=temp;			 
            setitimer(ITIMER_PROF, &(struct itimerval){.it_value = {0, runqueues[i].quantum * 1000}}, NULL);
            curr->tcb.state = RUNNING;	
			tot_cntx_switches++;		
            swapcontext(&scontext, &curr->tcb.context);
        }
    }
    
    // If all queues are empty, exit
    exit(0);
    
	
	
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
        return -1;
    }
    
    // Allocate a new stack for the new thread
    new_tcb->tstack = (char*) malloc(STACK_SIZE);
    if (new_tcb->tstack == NULL) {
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
		timerSet();
	}
	
    // Add the new TCB to the end of the linked list of TCBs
    struct node *new_node = (struct node*) malloc(sizeof(struct node));
    if (new_node == NULL) {
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
	tot_cntx_switches++;
    setcontext(&scontext);
    return 0;
}


/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
//find best fit for thread to pass context to 
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context
	// YOUR CODE HERE
	
	
	running->tcb.state=READY;
	tot_cntx_switches++;
	running->tcb.quantums++;
	swapcontext(&running->tcb.context, &scontext);
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE
	
	
	if((running==NULL)||(running->tcb.state==TERMINATED)){
		
		print_app_stats();
		setcontext(&mcontext);
		
	}
	
		running->tcb.state = TERMINATED;
		free(running->tcb.tstack);
		/*if(running!=NULL){
			head = running->next;
		}else{
			//call main context becasue all threads have run?
			print_app_stats();
		}*/
		
	
	if(value_ptr!=NULL){
		running->tcb.retVal=value_ptr;
	}
	tot_cntx_switches++;
	setcontext(&scontext);
	
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	 // - wait for a specific thread to terminate
    // - de-allocate any dynamic memory created by the joining thread
  
    // YOUR CODE HERE
    while (running != NULL) {
        if (running->tcb.tid == thread) {
            if (running->tcb.state == TERMINATED) {
                // Deallocate any memory allocated by the terminated thread
                free(running->tcb.tstack);
                // Remove the node from the list
                if (running == head) {
                    head = running->next;
                } else {
                    struct node *prev = head;
                    while (prev->next != running) {
                        prev = prev->next;
                    }
                    prev->next = running->next;
                }
                free(running);
                return 0;
            } else {
                // Wait for the thread to terminate
                running->tcb.state = BLOCKED;
                keyHolder = running;
				tot_cntx_switches++;
                swapcontext(&running->tcb.context, &scontext);
				
                break;
            }
        }
        running = running->next;
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
		
	}else{	
		mutex->tcb = running->tcb;
		mutex->key = 0;
		
	}
	return 0;
};

void insertQueue(struct TCB tcb, struct node* queuehead) {
   // create a link
   struct node *new_node = (struct node*) malloc(sizeof(struct node));
   new_node->tcb = tcb;
 
    new_node->next = queuehead;
    new_node->prev = NULL;
 
   
    if (queuehead != NULL)
        queuehead->prev = new_node;
 
    
    queuehead = new_node;
   }


/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
		// new version with test and set 
		mutex->tcb=running->tcb;
		while(__sync_lock_test_and_set(&(mutex->key),1)){
			insertQueue(running->tcb,mutex->head);
			running->tcb.state=BLOCKED;
			tot_cntx_switches++;
			swapcontext(&running->tcb.context, &scontext);
		}
		
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.
	// YOUR CODE HERE
	//new version with queue
	if(mutex->tcb.tid!=running->tcb.tid){
		return;
	}else{
		for(struct node *head; head!=NULL; head=head->next){
			head->tcb.state=READY;
			insert(head->tcb);
		}
		tot_cntx_switches++;
		setcontext(&scontext);
	}
	
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

void timerSignalHandler(int signum){
	//check if we are in sched
	ucontext_t currentContext;
	
	getcontext(&currentContext);
	if(&currentContext==&scontext){
		

	}else{
		setcontext(&scontext);
	}
}

void timerSet(){
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timerSignalHandler;
	sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	

	// Set up what the timer should reset to after the timer goes off
	timer.it_interval.tv_usec = (QUANTUM*1000)%1000000; 
	timer.it_interval.tv_sec = QUANTUM/1000;
	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = (QUANTUM*1000)%1000000; 
	timer.it_value.tv_sec = QUANTUM/1000;

	// Set the timer up (start the timer)
	
	setitimer(ITIMER_PROF, &timer, NULL);
	
}