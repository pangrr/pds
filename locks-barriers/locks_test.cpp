#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#include "locks.h"
#include "barriers.h" // ensure threads start counting at nearly the same time
#include "hrtimer_x86.h"

using namespace std;

int counter, n_increase, n_thread;
bool each;
pthreadbase_barrier_t *barrier;

double start_time, finish_time;

tatasbf_lock_t tatasbf_lock;
tatas_lock_t tatas_lock;
tas_lock_t tas_lock;
ticket_lock_t *ticket_lock;
mcs_qnode_t **mcs_lock;





void *thread_work (void *_tid) 
{
    int tid = *(int*)_tid;

    pthreadbase_barrier_wait (barrier); // all threads created
    
/*  no lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier);

    if (each) {
        for (int i = 0; i < n_increase; i++)
            counter++;
    } else {
        for (int i = 0; i < n_increase/n_thread; i++) // n_increase >> n_thread
            counter++;
    }
    
    pthreadbase_barrier_wait (barrier); 

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("\tmilliseconds\n");
        printf ("none\t%f\n", (finish_time-start_time)*1000);
    }

    pthreadbase_barrier_wait (barrier);

/* tas lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start time get

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
    
    pthreadbase_barrier_wait (barrier); 

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tas\t%f\n", (finish_time-start_time)*1000);
    }
    
    pthreadbase_barrier_wait (barrier);

/* tatas lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start time get

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
    
    pthreadbase_barrier_wait (barrier); 

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tatas\t%f\n", (finish_time-start_time)*1000);
    }
    
    pthreadbase_barrier_wait (barrier);

/* tatasbf lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start time get

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
    
    pthreadbase_barrier_wait (barrier); 

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tatasbf\t%f\n", (finish_time-start_time)*1000);
    }

    pthreadbase_barrier_wait (barrier);

/* ticket lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start time get

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

    pthreadbase_barrier_wait (barrier); 

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("ticket\t%f\n", (finish_time-start_time)*1000);
    }
    
    pthreadbase_barrier_wait (barrier);
    
/* mcs lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start time get

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
    
    pthreadbase_barrier_wait (barrier); 

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("mcs\t%f\n", (finish_time-start_time)*1000);
    }
}









int main(int argc, char **argv) 
{
    // default global values
    n_thread = 10;
    n_increase = 1000;
    each = false;
    
    int c;
    while((c = getopt(argc, argv, "t:i:eh")) != -1) {
        switch(c) {
            case 't':
                n_thread = atoi(optarg);
                break;
            case 'i':
                n_increase = atoi(optarg);
                break;
            case 'e':
                each = true;
                break;
            case 'h':
                printf ("locks_test\n-t <number of threads> (default=10)\n-i <times to increase the counter> (default=1000)\n-e (each thread increase the counter i times, default=false)\n");
                return 0;
        }
    }


    barrier = pthreadbase_barrier_init (n_thread);

    // launch threads
    int *tids = new int[n_thread];
    pthread_t *threads = (pthread_t*) malloc (sizeof(pthread_t) * n_thread);
    
    tas_lock_init (&tas_lock);
    tatas_lock_init (&tatas_lock);
    tatasbf_lock_init (&tatasbf_lock);
    ticket_lock = ticket_lock_init ();
    mcs_lock = mcs_lock_init ();
        

    for (int i = 0; i < n_thread; i++) {
        tids[i] = i;
        pthread_create (&threads[i], NULL, thread_work, (void*)&tids[i]);
    }
    

    for (int i = 0; i < n_thread; i++)
        pthread_join(threads[i], NULL);
}
