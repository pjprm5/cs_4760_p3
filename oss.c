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




int main (int argc, char *argv[])
{
  fprintf(stderr, "oss.c begins...\n");

  int option;                    // Holds option values.
  int maxChildSpawned = 5;       // Option -c; Holds max # of children to be spawned.
  int timeTillTerminate = 20;    // Option -t; Holds time till master terminates itself and all children.
  char *log;                     // Option -l; buffer
  char logReal[25] = {0};        // Option -l; Holds log filename.
  int logLength = 0;             // Testing purposes
  int i;
  
  int shmPID = 0;
  unsigned long int secs = 0;
  unsigned long int nanosecs = 0;


  

 

  // Getopt Menu loop.
  while ((option = getopt (argc, argv, "hc:t:l:" )) != -1)
  {
    switch (option)
    {
      case 'h' :
        fprintf(stderr, "Usage: ./oss -c # -t # -l str\n");
        fprintf(stderr, "-c decides max child processes to be spawned. Defaults to 5 if not specified.\n");
        fprintf(stderr, "-t decides the time in seconds until the master terminates all children and itself.\n");
        fprintf(stderr, "-l is the name of the log file being used to document the child processes exiting.\n");

        return -1;
      
      case 'c' :
        maxChildSpawned = atoi(optarg);
        if (maxChildSpawned > 20 || maxChildSpawned < 1)
        {
          fprintf(stderr, "Error: -c option must be from numbers 1-20. \n");
        }
        else
        {
          break;
        }

      case 't' :
        timeTillTerminate = atoi(optarg);
        if (timeTillTerminate > 60 || timeTillTerminate < 1)
        {
          fprintf(stderr, "Error: -t option must be from numbers 1-60. \n");
        }
        else
        {
          break;
        }

      case 'l' :
        logLength = strlen(optarg);
        log = malloc(strlen(optarg));
        strcpy(log, optarg);
        strncpy(logReal, log, 25);
        free(log);
        break;
  
      default:
        fprintf(stderr, "Default error. \n");
        return -1;
    }

  }

  fprintf(stderr,"maxChildSpawned: %d \n", maxChildSpawned);
  fprintf(stderr, "timeTillTerminate: %d \n", timeTillTerminate);
  //fprintf(stderr, "Log filename: %s \n", log);
  fprintf(stderr, "Log filename length: %d \n", logLength);
  fprintf(stderr, "Log Real: %s \n", logReal);
  
  // Allocate shared memory clock
/*
  key_t key = ftok("./oss", 'j');

  if ((shmid = shmget(key, 1024, 0600 | IPC_CREAT)) == -1)
  {
    perror("OSS: Error: shmget");
    exit(1);
  }*/
  




  return 0;
}
