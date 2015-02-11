#ifndef LOCKS_H__
#define LOCKS_H__

#include <stdio.h>
#include <stdlib.h>

#include "atoms.h"


////////////////////////////////////////
// tatas lock with backoff

typedef volatile unsigned long tatasbf_lock_t;

void tatasbf_lock_init (tatasbf_lock_t *L);

void tatasbf_lock_acquire_slowpath(tatasbf_lock_t* L);

void tatasbf_lock_acquire(tatasbf_lock_t* L);

void tatasbf_lock_release(tatasbf_lock_t* L);




////////////////////////////////////////
// tatas lock

typedef volatile unsigned long tatas_lock_t;

void tatas_lock_init (tatas_lock_t *L);

void tatas_lock_acquire (tatas_lock_t* L);

void tatas_lock_release (tatas_lock_t* L);



////////////////////////////////////////
// tas lock

typedef volatile unsigned long tas_lock_t;

void tas_lock_init(tas_lock_t *L); 

void tas_lock_acquire(tas_lock_t *L);

void tas_lock_release(tas_lock_t *L);



////////////////////////////////////////
// ticket lock

typedef struct ticket_lock_struct
{
    volatile unsigned long next_ticket;
    volatile unsigned long now_serving;
} ticket_lock_t;

ticket_lock_t* ticket_lock_init ();

void ticket_lock_acquire (ticket_lock_t *L);

void ticket_lock_release (ticket_lock_t *L);




////////////////////////////////////////
// MCS lock

typedef volatile struct mcs_qnode_struct
{
    bool flag;
    volatile struct mcs_qnode_struct *next;
} mcs_qnode_t;

mcs_qnode_t* mcs_qnode_init ();

mcs_qnode_t** mcs_lock_init ();

void mcs_lock_acquire (mcs_qnode_t **L, mcs_qnode_t *I);

void mcs_lock_release (mcs_qnode_t **L, mcs_qnode_t *I);

#endif // LOCKS_H__
