#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        #ifdef DEBUG
                printf("enqueue\n");
        #endif
        
        
        if(q->size < MAX_QUEUE_SIZE) q->proc[q->size++] = proc;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        
        //check the process with max priority
        
        if(empty(q)){
                #ifdef DEBUG
                        printf("queue rong\n");
                #endif
                
                return NULL;
        }
        
        int index_of_max = 0;
        for(int i = 0; i<q->size;i++){
                if(q->proc[i]->priority < q->proc[index_of_max]->priority){
                        index_of_max = i;
                }
        }

        //extract ans
        struct pcb_t* ans = q->proc[index_of_max];


        //remove out of the queue
        for(int i=index_of_max;i<q->size-1;i++){
                q->proc[i] = q->proc[i+1];
        }

        //increase the slot and decrease the size
        q->slot++;
        q->size--;

        

        //return the anser
	return ans;
}

