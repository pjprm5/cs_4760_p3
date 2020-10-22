// Paul Passiglia
// cs_4760
// Project 3
// 10/16/2020
// Struct for shared memory

#ifndef SHAREDCLOCK_H
#define SHAREDCLOCK_H

typedef struct {
  int secs;         // Holds secs
  int nanosecs;     // Holds nanosecs
  int secsKill;     // Holds term time for secs
  int nanosecsKill; // Holds term time for nanosecs
  int shmPID;       // Holds pid of terminated child process.
} SharedClock;

#endif
