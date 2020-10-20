// Paul Passiglia
// cs_4760
// Project 3
// 10/16/2020
// Struct for shared memory



#ifndef SHAREDCLOCK_H
#define SHAREDCLOCK_H

#include <semaphore.h>

typedef struct {
  int secs;        // Holds secs
  int nanosecs;    // Holds nanosecs
  //int secTerm;     // Holds secs of child to be terminated
  //int nanoTerm;    // Holds nanosecs of child to be terminated
  int shmPID;      // Holds pid of terminated child process.
  sem_t mutex;     // crit section sem
} SharedClock;

#endif
