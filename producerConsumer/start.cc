/* IMPORTANT START.CC CLEANS UP SEMAPHORES AND SHARED MEMORY */
/* MUST BE RUN AS A BACKGROUND PROCESS !!!!!*/

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>  
#include "helper.h"

int main(int argc, char **argv) {

  const int SEM_COUNT = 3;
  key_t sem_key = SEM_KEY, shm_key = SHM_KEY;
  int semid, shmid;
  queue *shared_queue; 

  //create shared memory
  shmid = shmget(shm_key, SHM_SIZE, 0666 | IPC_CREAT);

  //get pointer to shared memory
  shared_queue = (queue *) shmat(shmid, (void *)0, 0);
  if (shared_queue == (queue *) (-1)) {
    perror("Error attaching to shared memory\n");
    return(1);
  }
  
  //initalise shared queue
  shared_queue->prod_start_count = 0;
  shared_queue->cons_start_count = 0;
  shared_queue->prod_end_count = 0;
  shared_queue->cons_end_count = 0;
  int queue_size = check_arg(argv[1]);
  shared_queue->size = queue_size;
  shared_queue->front = 0;
  shared_queue->end = 0;
  
  //create 3 semaphores
  semid = sem_create(sem_key, SEM_COUNT);
  if( semid == -1 ) {
    perror("Error creating semaphores\n");
    return(1);
  }

  //initalise 3 semaphores  
  //MUTEX
  if ( sem_init(semid, MUTEX_SEM_NUM, MUTEX_SEM_VAL) == -1 ) {
    perror("Error initalising the MUTEX semaphore\n");
    return(1);
  }
  //EMPTY
  if ( sem_init(semid, EMPTY_SEM_NUM, EMPTY_SEM_VAL) == -1 ) {
    perror("Error initalising the EMPTY semaphore\n");
    return(1);
  }
  //FULL
  if ( sem_init(semid, FULL_SEM_NUM, queue_size) == -1 ) {
    perror("Error initalising the FULL semaphore\n");
    return(1);
  }

  //reasonable wait time for all producer/consumer processes to initate
  sleep(10);

  //test to see that all consumer and producer processes that initated have ended
  while ((shared_queue->cons_start_count != shared_queue->cons_end_count)
	 || (shared_queue->prod_start_count != shared_queue->prod_end_count)) {
    sleep(1);
  }

  // remove the semaphores
  if(sem_close(semid) == -1) {    
    perror("Error removing the semaphore\n");
    return(1);
  }

  //detach shared memory segment
  if ( shmdt(shared_queue) == -1) {
    perror("Error detaching the shared memory\n");
    return(1);
  }

  //delete shared memeory segment (assuming nobody attached)
  shmctl(shmid, IPC_RMID, NULL);

 return 0;
}
