/**
 * Author: Jason Bensel
 *	   Tyler Miller
 *
 * Description: Lab 6 - Controlled Process Synchronization
 *
 * */
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define SIZE 16

int main (int argc, char** argv) 
{ 
   int status; 
   long int i, loop, temp, *shmPtr; 
   int shmId, semid; 
   pid_t pid;
   struct sembuf sbuff[2];

   //Check for valid command line arguments
   if(argc != 2){
	perror("No arguments supplied\n");
        exit(1);
   }
   if((semid = semget(IPC_PRIVATE, 1, 00600)) == -1){
        perror("Failed to create semaphore\n");
	exit(1);
   }
   
   printf("SEMID: %d\n", semid);

   if(semctl(semid, 0, SETVAL, 1) == -1){
        perror("Failed to initialize semaphore\n");
	exit(1);
   }

   // get value of loop variable (from command-line argument)
   loop = atoi(argv[1]);
   
   //Buffer to decrement semaphore
   sbuff[0].sem_num = 0;
   sbuff[0].sem_op = -1;
   sbuff[0].sem_flg = 0;

   //Buffer to increment semaphore
   sbuff[1].sem_num = 0;
   sbuff[1].sem_op = 1;
   sbuff[1].sem_flg = 0;
   
   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n"); 
      exit (1); 
   } 
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) { 
      perror ("can't attach\n"); 
      exit (1); 
   }

   shmPtr[0] = 0; 
   shmPtr[1] = 1;

   if (!(pid = fork())) { 
      
      for (i=0; i<loop; i++) {
 //       printf("Child waiting for write priveledges...\n");
	semop(semid, &sbuff[0], 1); //Wait for sem to = 0 and decrement
//	printf("Child entering CS..\n"); 
        // swap the contents of shmPtr[0] and shmPtr[1]
	temp = shmPtr[0];	//CRITICAL SECTION
	shmPtr[0] = shmPtr[1];	//CRITICAL SECTION
	shmPtr[1] = temp;	//CRITICAL SECTION
  //      printf("Child exiting CS..\n");
        semop(semid, &sbuff[1], 1); //Signal semaphore
	 
      } 
      if (shmdt (shmPtr) < 0) { 
         perror ("just can't let go\n"); 
         exit (1); 
      } 
      exit(0); 
   } 
   else{ 

      for (i=0; i<loop; i++) { 
   //     printf("Parent waiting for write priveledges...\n");
        semop(semid, &sbuff[0], 1); //Wait for sem to = 0 and decrement
//	printf("Parent entering CS...\n");
	// swap the contents of shmPtr[1] and shmPtr[0]
	temp = shmPtr[1];	//CRITICAL SECTION
	shmPtr[1] = shmPtr[0];	//CRITICAL SECTION
	shmPtr[0] = temp;       //CRITICAL SECTION
//	printf("Parent exiting CS..\n");
        semop(semid, &sbuff[1], 1); //Signal semaphore
    
     }
   }
	
   wait (&status); 
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);
   
   //Remove the semaphore referenced by semid
   printf("Removing semaphore\n");
   semctl(semid, 0, IPC_RMID);
   if (shmdt (shmPtr) < 0) { 
      perror ("just can't let go\n"); 
      exit (1); 
   } 
   if (shmctl (shmId, IPC_RMID, 0) < 0) { 
      perror ("can't deallocate\n"); 
      exit(1); 
   }

   return 0; 
} 
