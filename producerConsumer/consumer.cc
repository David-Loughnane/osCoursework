// Consumer
#include <time.h>
# include "helper.h"

int main (int argc, char *argv[]) {
  
  key_t shm_key = SHM_KEY;
  int semid, shmid;
  queue *shared_queue;

  time_t start = time(0);
  int seconds_past;

  int consumer_id;
  consumer_id = check_arg(argv[1]);

  //get access to shared memory
  shmid = shmget(shm_key, SHM_SIZE, 0666);

  //get pointer to shared memory
  shared_queue = (queue *) shmat(shmid, (void *)0, 0);
  
  if (shared_queue == (queue *) (-1)) {
    perror("Error attaching to shared memory\n");
    return(1);
  }

  //let shared memory know a new consumer has entered the system
  shared_queue->cons_start_count++;

  //attach to semaphore
  semid = sem_attach(SEM_KEY);
  if (semid == -1) {
    perror("Error attaching to the semaphore\n");
    return(1);
  }
  
  //continue will there are still jobs to consume (10 sec wait assumption)
  while(sem_timewait(semid, EMPTY_SEM_NUM, 10) != -1) {

    //attempt access to critical section
    sem_wait(semid, MUTEX_SEM_NUM);

    //read the job duration and id
    int job_duration = shared_queue->job[shared_queue->front].duration;
    int job_id = shared_queue->job[shared_queue->front].id;

    //increment the front index value
    shared_queue->front = ( (shared_queue->front) + 1) % shared_queue->size;

    //release resources on semaphores
    sem_signal(semid, MUTEX_SEM_NUM);
    sem_signal(semid, FULL_SEM_NUM);
    
    //announce commencement of job
    seconds_past = difftime(time(0), start);
    printf("Consumer(%i) time %i: Job id %i executing sleep duration %i\n", 
	   consumer_id, seconds_past, job_id, job_duration);

    sleep(job_duration);

    //announce job completion
    seconds_past = difftime(time(0), start);
    printf("Consumer(%i) time %i: Job id %i completed\n",
	   consumer_id, seconds_past, job_id);
  }

  //let shared memory know consumer is exiting the system
  shared_queue->cons_end_count++;

  //detach shared memory segment
  if ( shmdt(shared_queue) == -1) {
    perror("Error detaching the shared memory\n");
    return(1);
  }

  return 0;
}
