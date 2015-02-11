#include "locks.h"

////////////////////////////////////////
// tatas lock with backoff



void tatasbf_lock_init (tatasbf_lock_t *L)
{
    *L = 0;
}

void tatasbf_lock_acquire_slowpath(tatasbf_lock_t* L)
{
    int b = 64;

    do {
        backoff(&b);
    } while (tas(L));
}

void tatasbf_lock_acquire(tatasbf_lock_t* L)
{
    if (tas(L))
        tatasbf_lock_acquire_slowpath(L);
}

void tatasbf_lock_release(tatasbf_lock_t* L)
{
    *L = 0;
}




////////////////////////////////////////
// tatas lock

void tatas_lock_init (tatas_lock_t *L)
{
    *L = 0;
}

void tatas_lock_acquire (tatas_lock_t* L)
{
    do {
        while (*L);
    } while (tas(L));
}

void tatas_lock_release (tatas_lock_t* L)
{
    *L = 0;
}



////////////////////////////////////////
// tas lock

void tas_lock_init(tas_lock_t *L) 
{
    *L = 0;
}

void tas_lock_acquire(tas_lock_t *L)
{
    while (tas(L));
}

void tas_lock_release(tas_lock_t *L)
{
    *L = 0;
}




////////////////////////////////////////
// ticket lock

ticket_lock_t* ticket_lock_init ()
{
    ticket_lock_t *L = (ticket_lock_t*) malloc (sizeof (ticket_lock_t));
    L->next_ticket = 0;
    L->now_serving = 0;
    
    return L;
}


void ticket_lock_acquire (ticket_lock_t *L)
{
    unsigned long my_ticket = fai (&L->next_ticket);
    while (L->now_serving != my_ticket);
}

void ticket_lock_release (ticket_lock_t *L)
{
    L->now_serving += 1;
}




////////////////////////////////////////
// MCS lock

mcs_qnode_t* mcs_qnode_init ()
{
    mcs_qnode_t *I = (mcs_qnode_t *) malloc (sizeof (mcs_qnode_t));
    I->flag = false;
    I->next = 0;
    
    return I;
}

mcs_qnode_t**  mcs_lock_init ()
{
    mcs_qnode_t **L = (mcs_qnode_t **) malloc (sizeof (mcs_qnode_t*));
    return L;
}

void mcs_lock_acquire (mcs_qnode_t **L, mcs_qnode_t *I)
{
    I->next = 0;
    mcs_qnode_t *pred =
        (mcs_qnode_t*) swap ((volatile unsigned long*) L, (unsigned long) I); // L->next = I, return old L->next

    if (pred != 0) { // queue not empty, lock unavailable
        I->flag = true; 
        pred->next = I; // append to queue
        while (I->flag); // spin
    }
}

void mcs_lock_release (mcs_qnode_t **L, mcs_qnode_t *I)
{
    if (I->next == 0) {
        if (bool_cas ((volatile unsigned long*) L, (unsigned long) I, 0))
            return;
        while (I->next == 0); // spin
    }
    I->next->flag = false;
}
