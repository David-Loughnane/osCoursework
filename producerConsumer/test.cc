#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "helper.h"

# define MUTEX_SEM_NUM 0
# define EMPTY_SEM_NUM 1
# define FULL_SEM_NUM  2
# define MUTEX_SEM_VAL 1
# define EMPTY_SEM_VAL 0
# define FULL_SEM_VAL 10


const int SEM_COUNT = 3;
key_t sem_key = SEM_KEY, shm_key = SHM_KEY;
int semid, shmid;
queue *shared_queue;


int main(int argc, char **argv) {

  //create shared memory
  shmid = shmget(shm_key, SHM_SIZE, 0644 | IPC_CREAT);
  //get pointer to shared memory - 0 
  shared_queue = (queue *) shmat(shmid, (void *)0, 0);
  
  if (shared_queue == (queue *) (-1)) {
    printf("Error attaching to shared memory: %d\n", shmid);
  }


  //create 3 semaphores
  semid = sem_create(sem_key, SEM_COUNT);

  //initalise 3 semaphores  
  //MUTEX
  if ( sem_init(semid, MUTEX_SEM_NUM, MUTEX_SEM_VAL) == -1 ) {
    printf("Error initalising the MUTEX semaphore: %d\n", sem_key);
  }
  /*
  //MUTEX down - request resource
  sem_wait(semid, MUTEX_SEM_NUM);
  
  //MUTEX up - release resource
  sem_signal(semid, MUTEX_SEM_NUM);
  */
  //EMPTY
  if ( sem_init(semid, EMPTY_SEM_NUM, EMPTY_SEM_VAL) == -1 ) {
    printf("Error initalising the EMPTY semaphore: %d\n", sem_key);
  }
  /*
  //EMPTY down - request resource
  sem_wait(semid, EMPTY_SEM_NUM);

  //wait 10s for consumer if another job is created, else quit
  sem_time_wait(semid, EMPTY_SEM_NUM, 10);
  
  //EMPTY up - release resource
  sem_signal(semid, EMPTY_SEM_NUM);
  */
  //FULL
  if ( sem_init(semid, MUTEX_SEM_NUM, MUTEX_SEM_VAL) == -1 ) {
    printf("Error initalising the FULL semaphore: %d\n", sem_key);
  }
  /*
  //FULL down - request resource
  sem_wait(semid, FULL_SEM_NUM);
  
  //FULL up - release resource
  sem_signal(semid, FULL_SEM_NUM);
  */ 
 
  sleep(25);

  // remove the semaphores
  if(sem_close(semid) == -1) {    
    printf("Error removing the semaphore: %d\n", semid);
  }

  //detach shared memory segment
  if ( shmdt(shared_queue) == -1) {
    printf("Error detaching the shared memory\n");
  }

  //delete shared memeory segment (assuming nobody attached)
  shmctl(shmid, IPC_RMID, NULL);


 return 0;
}
