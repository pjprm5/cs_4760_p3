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

// Prototypes.
void raiseAlarm();
int setSeconds(int nanosecs);

// Global Variables.
SharedClock *clockShare;
int clockID;


int main (int argc, char *argv[])
{
  //signal(SIGALRM, raiseAlarm); 

  fprintf(stderr, "oss.c begins...\n");
 
  int option;                    // Holds option values.
  int maxChildSpawned = 0;       // Option -c; Holds max # of children to be spawned.
  int timeTillTerminate = 0;    // Option -t; Holds time till master terminates itself and all children.
  int logSize = 25;              // Log file name length, no one would go past 25 right.
  char *createLogFile = malloc((sizeof(char) * logSize));  // Holds logfile name
  
  // Option flags
  int c_flag = 0; 
  int t_flag = 0;
  int l_flag = 0;
    
  int i;
    
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
  
  // Allocate shared memory clock
  key_t clockKey = ftok("makefile", (759)); // ftok generates key.
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

