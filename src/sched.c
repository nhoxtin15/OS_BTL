
#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static sem_t queue_sem[MAX_PRIO];


#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    	int i ;

	for (i = 0; i < MAX_PRIO; i ++){
		mlq_ready_queue[i].size = 0;
		//newly added
		mlq_ready_queue[i].slot = 0;
		mlq_ready_queue[i].slotMax = MAX_PRIO - i;
	}
		
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	#ifdef SYNCHRONIZE_QUEUE
		pthread_mutex_lock(&queue_lock);
	#endif
	//check if the queue is full
	int full = 1;
	for(int i = 0; i < MAX_PRIO; i++){
		if(mlq_ready_queue[i].slot <= mlq_ready_queue[i].slotMax){
			full = 0;
			break;
		}
	}

	if(full){
		for(int i = 0; i < MAX_PRIO; i++){
			mlq_ready_queue[i].slot = 0;
		}
		
	}
	//find the highest not empty queue that have available slot
	int i  = 0;
	for(	/* int i = 0*/ ;

		(empty(&mlq_ready_queue[i]) || mlq_ready_queue[i].slot >= mlq_ready_queue[i].slotMax) 
		&& i < MAX_PRIO;
		i++
	){	
	}
	//if the queue is not found
	if(i==MAX_PRIO){
		//there are still process in higher priority queue
		
		//reset the slot
		for(i = 0; i < MAX_PRIO; i++){
			mlq_ready_queue[i].slot = 0;
		}

		//find the highest not empty queue that have available slot
		
		for(	i = 0;

			(empty(&mlq_ready_queue[i]) || mlq_ready_queue[i].slot >= mlq_ready_queue[i].slotMax) 
			&& i <= MAX_PRIO;

			i++
		){	
		}
	}

	//if the queue is still not found -> the queue is empty -> return NULL
	if(i==MAX_PRIO){
		//all queue is empty
		pthread_mutex_unlock(&queue_lock);
		
		return NULL;
	}

	//the queue is at i
	struct pcb_t* ans = dequeue(&mlq_ready_queue[i]);

	#ifdef SYNCHRONIZE_QUEUE
		pthread_mutex_unlock(&queue_lock);
		sem_post(&queue_sem[i]);
	#endif
	
	return ans;
}

void put_mlq_proc(struct pcb_t * proc) {
	
	#ifdef SYNCHRONIZE_QUEUE
	//check if the queue is full
	sem_wait(&queue_sem[proc->prio]);
	#endif

	#ifdef SYNCHRONIZE_QUEUE
		pthread_mutex_lock(&queue_lock);
	#endif

	enqueue(&mlq_ready_queue[proc->prio], proc);

	#ifdef SYNCHRONIZE_QUEUE
		pthread_mutex_unlock(&queue_lock);
	#endif
}

void add_mlq_proc(struct pcb_t * proc) {
	//check if the queue is full
	#ifdef SYNCHRONIZE_QUEUE
		sem_wait(&queue_sem[proc->prio]);

		pthread_mutex_lock(&queue_lock);
	#endif

	enqueue(&mlq_ready_queue[proc->prio], proc);
	
	#ifdef SYNCHRONIZE_QUEUE
		pthread_mutex_unlock(&queue_lock);
	#endif
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	return proc;
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


