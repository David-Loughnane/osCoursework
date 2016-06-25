// Producer
#include <time.h>
#include "helper.h"


int main (int argc, char *argv[]) {

  key_t shm_key = SHM_KEY;
  int semid, shmid;
  queue *shared_queue;
  
  time_t start = time(0);
  int seconds_past;

  int producer_id, num_jobs;
  producer_id = check_arg(argv[1]);
  num_jobs = check_arg(argv[2]);

  //get access to shared memory
  shmid = shmget(shm_key, SHM_SIZE, 0666 /*| IPC_CREAT*/);

  //get pointer to shared memory
  shared_queue = (queue *) shmat(shmid, (void *)0, 0);
  
  if (shared_queue == (queue *) (-1)) {
    perror("Error attaching to shared memory\n");
    return(1);
  }

  //let shared memory know a new producer has entered the system
  shared_queue->prod_start_count++;
 
  //attach to semaphore
  semid = sem_attach(SEM_KEY);
  if (semid == -1) {
    perror("Error attaching to the semaphore\n");
    return(1);
  }

  //PRODUCE JOBS
  for (int i = 1; i <= num_jobs; i++) {

    sem_wait(semid, FULL_SEM_NUM);
    sem_wait(semid, MUTEX_SEM_NUM);

    //assign id of job as index + 1
    shared_queue->job[shared_queue->end].id = shared_queue->end + 1;
    //assign duration of job as number between 2 and 7
    shared_queue->job[shared_queue->end].duration = ((i%6)+2);

    seconds_past = difftime(time(0), start);
    printf("Producer(%i) time %i: Job id %i duration %i\n", 
	   producer_id, seconds_past, shared_queue->end + 1,
	   ((i%6+2)) );

    //increment the end index value
    shared_queue->end = ( (shared_queue->end) + 1 ) % shared_queue->size;

    sem_signal(semid, MUTEX_SEM_NUM);
    sem_signal(semid, EMPTY_SEM_NUM);

    sleep((i%3)+2);
  }

  seconds_past = difftime(time(0), start);
  printf("Producer(%i) time %i: No more jobs to generate.\n", 
	   producer_id, seconds_past);
  //let shared memeory know producer is exiting the system
  shared_queue->prod_end_count++;

  //detach shared memory segment
  if ( shmdt(shared_queue) == -1) {
    perror("Error detaching the shared memory\n");
    return(1);
  }

  return 0;
}
