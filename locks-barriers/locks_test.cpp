#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <string>

#include "locks.h"
#include "hrtimer_x86.h"

using namespace std;

int counter, n_increase, n_thread;
bool each;

tatasbf_lock_t tatasbf_lock;
tatas_lock_t tatas_lock;
tas_lock_t tas_lock;
ticket_lock_t *ticket_lock;
mcs_qnode_t **mcs_lock;

void *tas_lock_thread (void *argv) 
{
    if (each) {
        for (int i = 0; i < n_increase; i++) {
            tas_lock_acquire(&tas_lock);
            counter++;
            tas_lock_release(&tas_lock);
        }
    } else {
        while (counter < n_increase) {
            tas_lock_acquire(&tas_lock);
            counter++;
            tas_lock_release(&tas_lock);
        }
    }
}


void *tatas_lock_thread (void *argv) 
{
    if (each) {
        for (int i = 0; i < n_increase; i++) {
            tatas_lock_acquire(&tatas_lock);
            counter++;
            tatas_lock_release(&tatas_lock);
        }
    } else {
        while (counter < n_increase) {
            tatas_lock_acquire(&tatas_lock);
            counter++;
            tatas_lock_release(&tatas_lock);
        }
    }
}


void *tatasbf_lock_thread (void *argv) 
{
    if (each) {
        for (int i = 0; i < n_increase; i++) {
            tatasbf_lock_acquire(&tatasbf_lock);
            counter++;
            tatasbf_lock_release(&tatasbf_lock);
        }
    } else {
        while (counter < n_increase) {
            tatasbf_lock_acquire(&tatasbf_lock);
            counter++;
            tatasbf_lock_release(&tatasbf_lock);
        }
    }
}


void *ticket_lock_thread (void *argv) 
{
    if (each) {
        for (int i = 0; i < n_increase; i++) {
            ticket_lock_acquire(ticket_lock);
            counter++;
            ticket_lock_release(ticket_lock);
        }
    } else {
        while (counter < n_increase) {
            ticket_lock_acquire(ticket_lock);
            counter++;
            ticket_lock_release(ticket_lock);
        }
    }
}


void *mcs_lock_thread (void *argv) 
{
    mcs_qnode_t *I = mcs_qnode_init ();

    if (each) {
        for (int i = 0; i < n_increase; i++) {
            mcs_lock_acquire(mcs_lock, I);
            counter++;
            mcs_lock_release(mcs_lock, I);
        }
    } else {
        while (counter < n_increase) {
            mcs_lock_acquire(mcs_lock, I);
            counter++;
            mcs_lock_release(mcs_lock, I);
        }
    }
}


void *no_lock_thread (void *argv)
{
    if (each) {
        for (int i = 0; i < n_increase; i++)
            counter++;
    } else {
        for (int i = 0; i < n_increase/n_thread; i++) // n_increase >> n_thread
            counter++;
    }
}



int main(int argc, char **argv) 
{
    char *_lock_type;

    int c;
    while((c = getopt(argc, argv, "l:t:i:e")) != -1) {
        switch(c) {
            case 't':
                n_thread = atoi(optarg);
                break;
            case 'l':
                _lock_type = optarg;
                break;
            case 'i':
                n_increase = atoi(optarg);
                break;
            case 'e':
                each = true;
                break;
        }
    }

    string lock_type (_lock_type);
    counter = 0;

    // launch threads
    pthread_t *threads = (pthread_t *) malloc (sizeof(pthread_t) * n_thread);
    
    double start_time = gethrtime_x86 ();
        
    if (lock_type.compare (TAS) == 0) {
        tas_lock_init (&tas_lock);

        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, tas_lock_thread, NULL);
    
    
    } else if (lock_type.compare (TATAS) == 0) {
        tatas_lock_init (&tatas_lock);

        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, tatas_lock_thread, NULL);
    
    
    } else if (lock_type.compare (TATASBF) == 0) {
        tatasbf_lock_init (&tatasbf_lock);

        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, tatasbf_lock_thread, NULL);
    
    
    } else if (lock_type.compare (TICKET) == 0) {
        ticket_lock = ticket_lock_init ();

        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, ticket_lock_thread, NULL);
    
    
    } else if (lock_type.compare (MCS) == 0) {
        mcs_lock = mcs_lock_init ();

        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, mcs_lock_thread, NULL);
    
    
    } else if (lock_type.compare (NONE) == 0) {
        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, no_lock_thread, NULL);

    
    } else {
        printf("Lock types include:\n%s\n%s\n%s\n%s\n%s\n", TAS, TATAS, TATASBF, TICKET, MCS);
        exit(-1);
    }

    for (int i = 0; i < n_thread; i++)
        pthread_join(threads[i], NULL);

    double finish_time = gethrtime_x86 ();

    printf ("%f milliseconds\n", (finish_time-start_time) * 1000);
}
