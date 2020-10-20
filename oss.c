// Paul Passiglia
// cs_4760
// Project 3
// 10/15/2020
// oss.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include "sharedclock.h"
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <pthread.h>

// Prototypes.
void raiseAlarm();
int setSeconds(int nanosecs);

// Global Variables.
SharedClock *clockShare;
int clockID;

// Message queue struct
struct MessageQueue {
  long mtype;
  char messBuff[1];
};


int main (int argc, char *argv[])
{
  signal(SIGALRM, raiseAlarm); 

  fprintf(stderr, "oss.c begins...\n");
  int proc_count = 0;            // Holds the current number of child processes that has been activated.
  int option;                    // Holds option values.
  int maxChildSpawned = 0;       // Option -c; Holds max # of children to be spawned.
  int timeTillTerminate = 0;    // Option -t; Holds time till master terminates itself and all children.
  int logSize = 25;              // Log file name length, no one would go past 25 right.
  char *createLogFile = malloc((sizeof(char) * logSize));  // Holds logfile name.

  struct timespec tim1, tim2;
  tim1.tv_sec = 0;
  tim1.tv_nsec = 10000L;
  
  // Option flags
  int c_flag = 0; 
  int t_flag = 0;
  int l_flag = 0;
    
  
    
  // Getopt Menu loop.
  while ((option = getopt (argc, argv, "hc:t:l:" )) != -1)
  {
    switch (option)
    {
      case 'h' :
        fprintf(stderr, "Usage: ./oss -c # -t # -l str\n");
        fprintf(stderr, "-c decides max child processes to be spawned. Defaults to 5 if not specified.\n");
        fprintf(stderr, "-t decides the time in seconds until the master terminates all children and itself.\n");
        fprintf(stderr, "-l is the name of the log file being used to document the child processes exiting. Default: termlog.\n");

        return 0;
      
      case 'c' :   // Grab user input for max children to spawn. Option -c
        if (atoi(optarg) == 0)
        {
          perror("OSS: Error: The -c option must be an integer from 1-20: \n");
          return -1;
        }
        maxChildSpawned = atoi(optarg);
        if (maxChildSpawned > 20 || maxChildSpawned < 1)
        {
          fprintf(stderr, "Error: The -c option must be an integer from 1-20. \n");
          return -1;
        }

        c_flag = 1; // Set -c option flag to 1.
        break;
  

      case 't' :   // Grab user input for termination time. Option -t
        if (atoi(optarg) == 0)
        {
          perror("OSS: Error: The -t option must be an integer from 1-100. \n");
          return -1;
        }
        timeTillTerminate = atoi(optarg);
        if (timeTillTerminate > 100 || timeTillTerminate < 1)
        {
          fprintf(stderr, "Error: The -t option must be an integer from  1-100. \n");
          return -1;
        }
        
        t_flag = 1; // Set -t option flag to 1.

      case 'l' :   // Grab user input output log file name. Option -l
        if ((optarg) == '\0')
        {
          perror("OSS: Error: The -l option must have a filename after it. \n");
          return -1;
        }
         
        strcpy(createLogFile, optarg);
        l_flag = 1; 
        break;

      case '?' :  // Catch unknown arguments.
        if (optopt == 'c')
        {
          fprintf(stderr, "Error: The -c option needs an integer from 1-20 after it. \n");
          return -1;
        }

        if (optopt == 't')
        {
          fprintf(stderr, "Error: The -t option needs an integer from 1-100 after it. \n");
          return -1;
        }
        
        if (optopt == 'l')
        {
          fprintf(stderr, "Error: The -l option needs a filename after it. \n");
          return -1;
        }

        else if (isprint (optopt))
        {
          fprintf(stderr, "Error:  Unknown option detected. \n");
          return -1;
        }

        else
        {
          fprintf(stderr, "Error: Unknown error from options detected. \n");
          return -1;
        }

      default:
        fprintf(stderr, "Default error. \n");
        return -1;
    }

  }

  // Set default values.

  if (c_flag == 0)
  {
    maxChildSpawned = 5;
  }
  
  if (t_flag == 0)
  {
    timeTillTerminate = 20;
  }

  if (l_flag == 0)
  {
    strcpy(createLogFile, "termlog");
  }

  // Solves a bug I couldn't figure out where -l was taking argument of -c or -t without even -l being specified.
  if (atoi(createLogFile) == timeTillTerminate || atoi(createLogFile) == maxChildSpawned)
  {
    strcpy(createLogFile, "termlog");
  }


  fprintf(stderr,"maxChildSpawned: %d \n", maxChildSpawned);
  fprintf(stderr, "timeTillTerminate: %d \n", timeTillTerminate);
  printf("Log file name: %s \n", createLogFile);

  // Allocate message queue --------------------------------------------
  
  struct MessageQueue messageQ;
  int msqID;
  key_t msqKey = ftok("user.c", 666);
  msqID = msgget(msqKey, 0644 | IPC_CREAT);
  if (msqKey == -1)
  {
    perror("OSS: Error: ftok failure msqKey");
    exit(-1);
  }

  messageQ.mtype = 1;
  
  strcpy(messageQ.messBuff, "1"); // put a msg in buffer

  //int len = strlen(messageQ.messBuff); //length for buffer in msgsnd
  if(msgsnd(msqID, &messageQ, 1, 0) == -1)
  {
    perror("OSS: Error: msgsnd ");
    exit(-1);
  }
  
  printf("OSS(0): Message Sent: %s \n", messageQ.messBuff); 
 
  /*
  int check = 0;
  while (check == 0)
  {
    msgrcv(msqID, &messageQ, sizeof(messageQ), 1, 0);
    if (strcmp(messageQ.messBuff, "1") == 0)
    {
      printf("OSS(1): In Critical Section -> MSG Received: %s \n", messageQ.messBuff);
      
      check = 1;
    }
    if (check == 1)
    {
      printf("OSS(1): Out of Critical Section\n");
      strcpy(messageQ.messBuff, "1");
      msgsnd(msqID, &messageQ, 1, 0);
    }
  }*/

  /*
  int check2 = 0;
  while (check2 == 0)
  {
    msgrcv(msqID, &messageQ, sizeof(messageQ), 1, 0);
    if (strcmp(messageQ.messBuff, "1") == 0)
    {
      printf("OSS(2): In Critical Section -> MSG Received: %s \n", messageQ.messBuff);
      check2 = 1;
    }
    if (check2 == 1)
    {
      printf("OSS(2): Out of Critical Section\n");
      strcpy(messageQ.messBuff, "1");
      msgsnd(msqID, &messageQ, 1, 0);
    }
  }
  */


  /*
  if(msgctl(msqID, IPC_RMID, NULL) == -1)
  {
    perror("OSS: Error: msgctl ");
  }
  */
  
  // Allocate shared memory clock --------------------------------------
  key_t clockKey = ftok("makefile", 123); // ftok generates key.
  if (clockKey == -1) 
  {
    perror("OSS: Error: ftok failure");
    exit(-1);
  }
  
  // Create shared memory ID
  clockID = shmget(clockKey, sizeof(SharedClock), 0600 | IPC_CREAT);
  if (clockID == -1)
  {
    perror("OSS: Error: shmget failure");
    exit(-1);
  }
  
  // Attach to shared memory
  clockShare = (SharedClock*)shmat(clockID, (void *)0, 0);
  if (clockShare == (void*)-1)
  {
    perror("OSS: Error: shmat failure");
    exit(-1);
  }
  //---------------------------------------------------------------------
  
  // Initialize shared clock values and shmPID to zero.
  clockShare->secs = 0;
  clockShare->nanosecs = 0;
  clockShare->shmPID = 0;

  // Child process logic
  sem_init(&(clockShare->mutex), 1, 1);
  alarm(timeTillTerminate);
  pid_t childPid;  // Child pid.
  
/*  alarm(timeTillTerminate); // Set termination alarm

  childPid = fork();
  if (childPid == 0)
   {
     char *args[] = {"./user", NULL};
     execvp(args[0], args); // Exec child process.
   }
  else if (childPid < 0)
  {
    printf("%s: ", argv[0]);
    perror("OSS: Error: Fork() failed. ");
  }
  else if (childPid > 0)
  {
    printf("Begin:\n+[%d](%d)+ -> \n ", childPid, proc_count);
  }
  */

  //wait(NULL);
  //kill(0, SIGKILL);
  
  
  int i;
  for (i = 0; i < maxChildSpawned; i++)
  {
    proc_count++;
    childPid = fork();
    if (childPid == 0)
    {
      char *args[] = {"./user", NULL};
      execvp(args[0], args);
    }
    else if (childPid < 0)
    {
      perror("Error: Fork() failed ");
    }
    else if ((childPid > 0) && (i >= 0))
    {
      printf("Begin:\n +[%d](%d)+ -> \n ", childPid, proc_count);
      FILE* fptr = (fopen(createLogFile, "a"));
      if (fptr == NULL)
      {
        perror("OSS: Error: fptr error: ");
      }
      fprintf(fptr, "OSS(%d)(process #%d): CREATING USER DEFINED INITIAL CHILD #(%d) with child pid: (%d) at my time: \n", getpid(), proc_count, proc_count, childPid);
      printf("OSS(%d)(process #%d): CREATING USER DEFINED INITIAL CHILD #(%d) with child pid: (%d) at my time : \n", getpid(), proc_count, proc_count, childPid);
      fclose(fptr);
    }
  }

  

  // Parent logic, main loop

  
  int guard = 0;
  while (guard == 0)
  {
    sem_wait(&(clockShare->mutex));
    clockShare->nanosecs = (clockShare->nanosecs + (100 * maxChildSpawned)); // begin simulated time with this increment based on a constant
    if (clockShare->nanosecs >= 1000000000)
    {
      int secsTemp = setSeconds(clockShare->nanosecs); // grabs number of seconds from nanosecs
      clockShare->secs = clockShare->secs + secsTemp;
      clockShare->nanosecs = clockShare->nanosecs % 1000000000; // take away secs from nanosecs
    }
    sem_post(&(clockShare->mutex));

    sem_wait(&(clockShare->mutex));

    if(clockShare->shmPID != 0)
    {
      //waitpid(clockShare->shmPID); // while a child is terminating, wait for it to die
      wait(NULL);

      FILE* fptr = fopen(createLogFile, "a");

      fprintf(fptr, "OSS(%d)(process #%d): Child pid: (%d) is terminating at system clock time: %09lf \n", getpid(), proc_count, clockShare->shmPID, (((double) clockShare->secs) + ((double) clockShare->nanosecs/1000000000)));
      
      printf("OSS(%d)(process #%d): Child pid: (%d) is terminating at system clock time: %09lf \n", getpid(), proc_count, clockShare->shmPID, (((double) clockShare->secs) + ((double) clockShare->nanosecs/1000000000)));

      fclose(fptr);

      clockShare->shmPID = 0; // shmPID is at zero so a new child can fight for it
      
      if (clockShare ->secs >= 2) // here we check for the simulated time reaching 2 seconds and then terminate
      {
        sem_post(&(clockShare->mutex)); 
        printf("The simulated time has reached 2, terminating all processess.\n");
        FILE* fptr = fopen(createLogFile, "a");
        fprintf(fptr, "The simulated time has reached 2, terminating all processes.\n");
        fclose(fptr);

        shmdt(clockShare);
        shmctl(clockID, IPC_RMID, NULL);
        kill(0, SIGKILL);
      }

      else
      {
        sem_post(&(clockShare->mutex));
      }

      int guard2 = 0;
      while (guard2 == 0) // Controls child replacement
      {
        if (proc_count < 100) // Replaces child if process limit has not been reached
        {
          guard2 = 1; // get out of loop
          while (nanosleep(&tim1, &tim2));
          childPid = fork();
          if (childPid == 0) // Spawn replacement child
          {
            char *args[] = {"./user", NULL};
            execvp(args[0], args);
          }
          else if (childPid < 0)
          {
            guard2 = 0; // stay in loop and retry
            while(nanosleep(&tim1, &tim2));
          }
          else if (childPid > 0)
          {

            proc_count++; // increment process counter since a replacement was spawned
            FILE* fptr = fopen(createLogFile, "a");
            fprintf(fptr, "OSS(%d)(process #%d): Creating new replacement child pid: (%d) at my time: \n", getpid(), proc_count, childPid);
            printf("OSS(%d)(process #%d): Creating new replacement child pid: (%d) at my time: \n", getpid(), proc_count, childPid);
            fclose(fptr);
          }
        }
        else // Terminate since 100 children have been spawned.
        {
          printf("\nMax number of processes (100) reached, all processes terminated\n");
          FILE* fptr = fopen(createLogFile, "a");
          fprintf(fptr, "Max number of processes (100) reached, all processes terminated\n");
          fclose(fptr);
          shmdt(clockShare);
          shmctl(clockID, IPC_RMID, NULL);
          kill(0, SIGKILL);
        }
      }
    }
    else
    {
      sem_post(&(clockShare->mutex));
    }
  }
  
 
  // Detach from shared memory.
  shmdt(clockShare);
  // Free shared memory.
  shmctl(clockID, IPC_RMID, NULL);

  return 0;
}



int setSeconds(int nanosecs)
{
  int give = 0;
  if (nanosecs >= 1000000000)
  {
    nanosecs = nanosecs - 1000000000;
    give = 1 + setSeconds(nanosecs);
  }
  return give;
}

void raiseAlarm()
{
  printf("\nTime limit hit, terminating all processes.\n");
  shmdt(clockShare);
  shmctl(clockID, IPC_RMID, NULL);
  kill(0, SIGKILL);
}

