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
#include <getopt.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "sharedclock.h"

// Prototypes.
void raiseAlarm();

// Global Variables.
SharedClock *clockShare;
int clockID;
int msqID;

// Message queue struct
struct MessageQueue {
  long mtype;
  char messBuff[1];
};

int main (int argc, char *argv[])
{
  signal(SIGALRM, raiseAlarm);   // Start alarm.

  int proc_count = 0;            // Holds the current number of child processes that has been activated.
  int option;                    // Holds option values.
  int maxChildSpawned = 0;       // Option -c; Holds max # of children to be spawned.
  int timeTillTerminate = 0;     // Option -t; Holds time till master terminates itself and all children.
  int logSize = 25;              // Log file name length, no one would go past 25 right.
  char *createLogFile = malloc((sizeof(char) * logSize));  // Holds logfile name.

  // Timespec struct for nanosleep.
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
        fprintf(stderr, "-t decides the time in seconds until the master terminates all children and itself. Defaults to 20 if not specified.\n");
        fprintf(stderr, "-l is the name of the log file being used to document the child processes exiting. Default: termlog.\n");
        fprintf(stderr, "./oss forks of the multiple child processes defined from -c option. These processes hang around for a random time then terminate.\n");
        fprintf(stderr, "Everytime a child process terminates it is logged into the output file defined from -l option, then ./oss forks a replacement child.\n");
        fprintf(stderr, "This continues until 100 child processes have launched or until 2 seconds is reached in simulated time.\n");
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
        l_flag = 1; // Set -l flag to 1. 
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
  fprintf(stderr, "Log file name: %s \n", createLogFile);

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

  messageQ.mtype = 2;
  
  strcpy(messageQ.messBuff, "1"); // put a msg in buffer

  
  if(msgsnd(msqID, &messageQ, 1, 0) == -1)
  {
    perror("OSS: Error: msgsnd ");
    exit(-1);
  }
  
  // Allocate shared memory clock --------------------------------------
  key_t clockKey = ftok("makefile", 123); // ftok generates key.
  if (clockKey == -1) 
  {
    perror("OSS: Error: ftok failure");
    exit(-1);
  }
  
  // Create shared memory ID.
  clockID = shmget(clockKey, sizeof(SharedClock), 0600 | IPC_CREAT);
  if (clockID == -1)
  {
    perror("OSS: Error: shmget failure");
    exit(-1);
  }
  
  // Attach to shared memory.
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
  alarm(timeTillTerminate);
  pid_t childPid;  // Child pid.
  
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
      FILE* fptr = (fopen(createLogFile, "a"));
      if (fptr == NULL)
      {
        perror("OSS: Error: fptr error: ");
      }
      fprintf(fptr, "OSS(%d)(process #%d): CREATING USER DEFINED INITIAL CHILD #(%d) with child pid: (%d). \n", getpid(), proc_count, proc_count, childPid);
      printf("OSS(%d)(process #%d): CREATING USER DEFINED INITIAL CHILD #(%d) with child pid: (%d). \n", getpid(), proc_count, proc_count, childPid);
      fclose(fptr);
    }
  }

  // Parent logic, main loop.
  while (1)
  {   
    
    msgrcv(msqID, &messageQ, sizeof(messageQ.messBuff), 1, 0); // msgrcv blocks for crit section, only accepting an mtype of 1. The ./oss only accepts mtype of 1.
    clockShare->nanosecs = (clockShare->nanosecs + (100 * maxChildSpawned)); // Begin simulated time with this increment based on a constant.
    if (clockShare->nanosecs >= 1000000000) // Fix the clock each iteration for rollover protection.
    {          
      clockShare->secs = clockShare->secs + clockShare->nanosecs/1000000000; // Add seconds to nanosecs after nanosecs gets integer division.
      clockShare->nanosecs = clockShare->nanosecs % 1000000000; // Modulo operation to get nanosecs.
    }

    if (clockShare->shmPID != 0) // If the shmPID is not zero, we go in and wait for child to die.
    {
      wait(NULL);

      if (clockShare->secs >= 2) // Here we check for the simulated time reaching 2 seconds, write to log, terminate.
      {
        printf("The simulated time has reached 2, terminating all processess.\n");
        FILE* fptr = fopen(createLogFile, "a");
        fprintf(fptr, "The simulated time has reached 2, terminating all processes.\n");
        fclose(fptr);
        shmdt(clockShare);               // Detach clockShare.
        shmctl(clockID, IPC_RMID, NULL); // Destroy clockShare.
        msgctl(msqID, IPC_RMID, NULL);   // Destory message queue.
        kill(0, SIGKILL);                // Kill processes.
      }
     
      FILE* fptr = fopen(createLogFile, "a");
      fprintf(fptr, "OSS(%d)(process #%d): Child pid: (%d) is terminating at system clock time: %09lf \n", getpid(), proc_count, clockShare->shmPID, (((double) clockShare->secsKill) + ((double) clockShare->nanosecsKill/1000000000)));
      printf("OSS(%d)(process #%d): Child pid: (%d) is terminating at system clock time: %09lf \n", getpid(), proc_count, clockShare->shmPID, (((double) clockShare->secsKill) + ((double) clockShare->nanosecsKill/1000000000)));
      fclose(fptr);

      clockShare->shmPID = 0; // shmPID now equals zero to signal another child process.

      if (proc_count < 100) // Replace child processes if limit not reached.
      {
      
        childPid = fork();
        if (childPid == 0) // Spawn replacement child.
        {
          char *args[] = {"./user", NULL};
          execvp(args[0], args);
        }

        else if (childPid < 0)
        {
          // Stay in loop and retry with a little nanosleep.
          while(nanosleep(&tim1, &tim2));
        }
     
        else if (childPid > 0) // Child successfully spawned, then is logged.
        {
          proc_count++; // Increment process counter since a replacement was spawned.
          FILE* fptr = fopen(createLogFile, "a");
          fprintf(fptr, "OSS(%d)(process #%d): Creating new replacement child pid: (%d) at my time: %09lf \n", getpid(), proc_count, childPid, (((double) clockShare->secs) + ((double) clockShare->nanosecs/1000000000)));
          printf("OSS(%d)(process #%d): Creating new replacement child pid: (%d) at my time: %09lf \n", getpid(), proc_count, childPid, (((double) clockShare->secs) + ((double) clockShare->nanosecs/1000000000)));
          fclose(fptr);
        }
      }
      else // 100 processes was reached, log, kill everything.
      {
        printf("\nMax number of processes (100) reached, all processes terminated\n");
        FILE* fptr = fopen(createLogFile, "a");
        fprintf(fptr, "Max number of processes (100) reached, all processes terminated\n");
        fclose(fptr);
        shmdt(clockShare);               // Detach clockShare
        shmctl(clockID, IPC_RMID, NULL); // Destroy clockShare
        msgctl(msqID, IPC_RMID, NULL);   // Destroy message queue
        kill(0, SIGKILL);                // Kill processes.
      }
      
    }
    // Msgsnd signals with an mtype of 2 to alert a child process. From ./oss only mtype of 2 is sent out.
    messageQ.mtype = 2;
    msgsnd(msqID, &messageQ, sizeof(messageQ.messBuff), 0);
    
  }
  // Code below ideally should not execute. 
  // Detach from shared memory.
  shmdt(clockShare);
  // Free shared memory.
  shmctl(clockID, IPC_RMID, NULL);
  msgctl(msqID, IPC_RMID, NULL);

  return 0;
}

void raiseAlarm() // Kill everything if user input of time limit is reached.
{
  printf("\nTime limit hit, terminating all processes.\n");
  shmdt(clockShare);
  shmctl(clockID, IPC_RMID, NULL);
  msgctl(msqID, IPC_RMID, NULL);
  kill(0, SIGKILL);
}

