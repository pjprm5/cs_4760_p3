// Paul Passiglia
// cs_4760
// Project 3 
// 10/15/2020
// User.c


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


int randomTime()
{
  int randomNum = ((rand() % (1000000 - 1 + 1)) +1);
  return randomNum;
}

// Message queue struct
struct MessageQueue {
  long mtype;
  char messBuff[512];
};


SharedClock *clockShare;
int clockID;

int main (int argc, char *argv[])
{
  printf("\nuser.c begins...\n");

  // Allocate shared memory clock
  key_t clockKey = ftok("makefile", 777);
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


  

  // Receive Message and print.
  /*  
  int check = 0;
  while (1)
  {
    msgrcv(msqID, &messageQ, sizeof(messageQ)+1, 1, IPC_NOWAIT);
    if (strcmp(messageQ.messBuff, "1") == 0)
    {
      printf("USER(1): In Critical Section -> MSG Received: %s \n", messageQ.messBuff);
      check = 1;
    }
   
    if(check == 1)
    {
      printf("USER(1) Out of Critical Section \n");
      strcpy(messageQ.messBuff, "1");
      msgsnd(msqID, &messageQ, 1, 0);
      break;
    }
    
  }
  
  int check2 = 0;
  while (1)
  {
    msgrcv(msqID, &messageQ, sizeof(messageQ)+1, 1, IPC_NOWAIT);
    if (strcmp(messageQ.messBuff, "1") == 0)
    {
      printf("USER(2): In Critical Section -> MSG Received: %s \n", messageQ.messBuff);
      check2 = 1;
    }
    if (check2 == 1)
    {
      printf("USER(2): Out of Critical Section\n");
      strcpy(messageQ.messBuff, "1");
      msgsnd(msqID, &messageQ, 1, 0);
      break;
    }
  }
  */




 /* 
  if (msgrcv(msqID, &messageQ, sizeof(messageQ.messBuff), 1, 0) == -1)
  {
    perror("USER: Error: msgrcv failure ");
    exit(-1);
  }
  
  //printf("TESTING...\n");
  printf("USER: Message Received: %s \n", messageQ.messBuff);
  

  // Change Message and Send
  
  strcpy(messageQ.messBuff, "7");
  int len = strlen(messageQ.messBuff);
  if(msgsnd(msqID, &messageQ, len+1, 0) == -1)
  {
    perror("USER: Error: msgsnd ");
    exit(-1);
  }
  printf("USER: Message Sent: %s \n", messageQ.messBuff);
  

  int currentTime = (clockShare->secs * 1000000000) + clockShare->nanosecs;

  printf("\nCurrent time: %d \n", currentTime);
  printf("Seconds: %d \n", clockShare->secs);
  printf("Nanoseconds: %d \n", clockShare->nanosecs);
  shmdt(clockShare); // Detach child from shared memory.
  */

  //message_wait();
  int termTime = (clockShare->secs * 1000000000) + clockShare->nanosecs;
  //message_post();
  termTime = termTime + randomTime();
  int guard = 0;
  while (guard == 0)
  {
    //message_wait();
    int tempTime = (clockShare->secs * 1000000000) + clockShare->nanosecs; // Grab current time
    //message_post();
    while (tempTime >= termTime)
    {
      //message_wait();
      if (clockShare->shmPID == 0)
      {
        clockShare->secTerm = clockShare->secs;
        clockShare->nanoTerm = clockShare->nanosecs;
        clockShare->shmPID = getpid();
        //message_post();
        shmdt(clockShare);
        return 0;
      }
      else
      {
        //message_post();
      }
    }
  }
  return 0;
}
