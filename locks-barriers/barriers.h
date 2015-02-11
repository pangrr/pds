#ifndef BARRIERS_H__
#define BARRIERS_H__

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "atoms.h" // atomic operations

// thread local sense
typedef struct tid_sense_struct
{
    bool sense;
    int tid;
} tid_sense_t;

tid_sense_t* tid_sense_init (int n_thread);


/////////////////////////////////////
// centralized sense-reversing barrier

#define CS "cs"

typedef struct cs_barrier_struct
{
    unsigned long arrived;
    bool sense;
    int n_thread;
} cs_barrier_t;

cs_barrier_t* cs_barrier_init (int n_thread);

void cs_barrier_wait (cs_barrier_t *barrier, bool *local_sense);







//////////////////////////////////////
// tournament barrier

#define TOURNAMENT "tournament"

#define WINNER 0
#define LOSER 1
#define BYE 2
#define CHAMPION 3
#define DROPOUT 4
#define DEFAULT -1

typedef struct round_struct 
{
	int role;
	bool *opponent;
	bool flag;
} round_t;

typedef struct tournament_barrier_struct
{
    round_t **array;
    bool opponent_default;
    int n_round;
} tournament_barrier_t;


tournament_barrier_t* tournament_barrier_init (int n_thread); 

void tournament_barrier_wait (tournament_barrier_t *barrier, int tid, bool *sense); 







///////////////////////////////////////////
// pthread-based condition synchronization
// Extracted from sor_pthread.c

#define PTHREAD "pthread"

typedef struct pthreadbase_barrier_struct
{
    int n_thread;
    int arrived;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

} pthreadbase_barrier_t;


pthreadbase_barrier_t* pthreadbase_barrier_init (int n_thread);


void pthreadbase_barrier_wait (pthreadbase_barrier_t *barrier);

#endif // BARRIERS_H__

