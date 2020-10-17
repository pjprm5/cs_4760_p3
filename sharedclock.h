// Paul Passiglia
// cs_4760
// Project 3
// 10/16/2020
// Struct for shared memory



#ifndef SHAREDCLOCK_H
#define SHAREDCLOCK_H

typedef struct {
  int sec;     // Holds secs
  int nano;    // Holds nanosecs
  int shmPID;  // Holds pid of terminated child process.
} SharedClock;

#endif
