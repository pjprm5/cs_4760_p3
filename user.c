// Paul Passiglia
// cs_4760
// Project 3 
// 10/15/2020
// user.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include "sharedclock.h"

int randomTime() // Grad random number for child process ttl.
{
  int randomNum = ((rand() % (10000000 - 1 + 1)) +1);
  return randomNum;
}

// Message queue struct
struct MessageQueue {
  long mtype;
  char messBuff[1];
};
// Global variables
SharedClock *clockShare;
int clockID;

int main (int argc, char *argv[])
{
    
  // Allocate shared memory clock
  key_t clockKey = ftok("makefile", 123);
  if (clockKey == -1)
  {
    perror("USER: Error: ftok failure");
    exit(-1);
  }

  // Create shared memory ID
  clockID = shmget(clockKey, sizeof(SharedClock), 0600 | IPC_CREAT);
  if (clockID == -1)
  {
    perror("USER: Error: shmget failure");
    exit(-1);
  }

  //Attach to shared memory
  clockShare = (SharedClock*)shmat(clockID, (void *)0, 0);
  if (clockShare == (void*)-1)
  {
    perror("USER: Error: shmat failure");
    exit(-1);
  }
  
  // Allocate Message Queue
  
  struct MessageQueue messageQ;
  int msqID;
  key_t msqKey = ftok("user.c", 666);
  msqID = msgget(msqKey, 0644 | IPC_CREAT);
  if (msqKey == -1)
  {
    perror("USER: Error: ftok failure msqKey");
    exit(-1);
  }
  
  // Being main loop
  int readSec = (clockShare->secs);      // Read sec
  int readNano = (clockShare->nanosecs); // Read nanosecs
  int secsToEnd = readSec;               // Setup secs to end
  int nanosecsToEnd = readNano;          // Setup nanosecs to end
  int addRand = randomTime();            // Get a random time.

  if (readNano + addRand >= 1000000000)  // Check rollovers 
  {
    secsToEnd += (readNano + addRand)/1000000000;
    nanosecsToEnd = (readNano + addRand) % 1000000000;
  }


  while (1)
  {   
    // msgrcv waits to receive only mtype of 2. ./user only receives mtype of 2.
    if (msgrcv(msqID, &messageQ, sizeof(messageQ.messBuff), 2, 0) == 1)
    {
      if (clockShare->shmPID == 0 && clockShare->secs >= secsToEnd)  
      {
        if (clockShare->nanosecs >= nanosecsToEnd)
        {
          clockShare->secsKill = clockShare->secs;           // Grab term time for secs
          clockShare->nanosecsKill = clockShare->nanosecs;   // Grab term time for nanosecs
          clockShare->shmPID = getpid();                     // Give child pid to clockShare
          //printf("USER: In CS: %d \n", clockShare->shmPID);
          messageQ.mtype = 1;
          msgsnd(msqID, &messageQ, sizeof(messageQ.messBuff), 0);
          // msgsnd sends out with a mtype of 1. ./user only ever sends out with mtype of 1.
          shmdt(clockShare);
          return 0;    
        }
      }
    }
    messageQ.mtype = 1;
    msgsnd(msqID, &messageQ, sizeof(messageQ.messBuff), 0); 
    // msgsnd sends out with a mtype of 1. ./user only ever sends out with mtype of 1.
      
  }
  return 0;
}
