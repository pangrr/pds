#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#include "locks.h"
#include "barriers.h" // ensure threads start counting at nearly the same time
#include "hrtimer_x86.h"

using namespace std;

unsigned long counter, n_increase;
int n_thread;
bool each;
pthreadbase_barrier_t *barrier;

double start_time, finish_time;

tatasbf_lock_t tatasbf_lock;
tatas_lock_t tatas_lock;
tas_lock_t tas_lock;
ticket_lock_t *ticket_lock;
mcs_qnode_t **mcs_lock;

unsigned long *none_perthread_increase, *tas_perthread_increase, *tatas_perthread_increase, *tatasbf_perthread_increase, *ticket_perthread_increase, *mcs_perthread_increase, *fai_perthread_increase;



void *thread_work (void *_tid) 
{
    int tid = *(int*)_tid;
    int increase = 0; // how much does this thread contribute

    pthreadbase_barrier_wait (barrier); // all threads created
    
/*  no lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    if (each) {
        for (unsigned long i = 0; i < n_increase; i++)
            counter++;
    } else {
        while (counter < n_increase) { // who can beat me
            counter++;
            increase++;
        }
    }
    
    pthreadbase_barrier_wait (barrier); // end racing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("\n\ttime/milliseconds\tcounter\n");
        printf ("none\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }

    none_perthread_increase[tid] = increase;
    increase = 0;

    pthreadbase_barrier_wait (barrier);

/* tas lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    if (each) {
        for (unsigned long i = 0; i < n_increase; i++) {
            tas_lock_acquire(&tas_lock);
            counter++;
            tas_lock_release(&tas_lock);
        }
    } else {
        while (1) {
            tas_lock_acquire(&tas_lock);
            if (counter < n_increase) {
                counter++;
                increase++;
                tas_lock_release(&tas_lock);
            } else {
                tas_lock_release(&tas_lock);
                break;
            }
        }
    }
    
    pthreadbase_barrier_wait (barrier); // end racing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tas\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }
    
    tas_perthread_increase[tid] = increase;
    increase = 0;
    
    pthreadbase_barrier_wait (barrier);

/* tatas lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    if (each) {
        for (unsigned long i = 0; i < n_increase; i++) {
            tatas_lock_acquire(&tatas_lock);
            counter++;
            tatas_lock_release(&tatas_lock);
        }
    } else {
        while (1) {
            tatas_lock_acquire(&tatas_lock);
            if (counter < n_increase) {
                counter++;
                increase++;
                tatas_lock_release(&tatas_lock);
            } else {
                tatas_lock_release(&tatas_lock);
                break;
            }
        }
    }
    
    pthreadbase_barrier_wait (barrier); // end racing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tatas\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }
    
    tatas_perthread_increase[tid] = increase;
    increase = 0;
    
    pthreadbase_barrier_wait (barrier);

/* tatasbf lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    if (each) {
        for (unsigned long i = 0; i < n_increase; i++) {
            tatasbf_lock_acquire(&tatasbf_lock);
            counter++;
            tatasbf_lock_release(&tatasbf_lock);
        }
    } else {
        while (1) {
            tatasbf_lock_acquire(&tatasbf_lock);
            if (counter < n_increase) {
                counter++;
                increase++;
                tatasbf_lock_release(&tatasbf_lock);
            } else {
                tatasbf_lock_release(&tatasbf_lock);
                break;
            }
        }
    }
    
    pthreadbase_barrier_wait (barrier); // end reacing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tatasbf\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }

    tatasbf_perthread_increase[tid] = increase;
    increase = 0;
    
    pthreadbase_barrier_wait (barrier);

/* ticket lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    if (each) {
        for (unsigned long i = 0; i < n_increase; i++) {
            ticket_lock_acquire(ticket_lock);
            counter++;
            ticket_lock_release(ticket_lock);
        }
    } else {
        while (1) {
            ticket_lock_acquire(ticket_lock);
            if (counter < n_increase) {
                counter++;
                increase++;
                ticket_lock_release(ticket_lock);
            } else {
                ticket_lock_release(ticket_lock);
                break;
            }
        }
    }

    pthreadbase_barrier_wait (barrier); // end racing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("ticket\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }
    
    ticket_perthread_increase[tid] = increase;
    increase = 0;
    
    pthreadbase_barrier_wait (barrier);
    
/* mcs lock */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    mcs_qnode_t *I = mcs_qnode_init ();
    
    if (each) {
        for (unsigned long i = 0; i < n_increase; i++) {
            mcs_lock_acquire(mcs_lock, I);
            counter++;
            mcs_lock_release(mcs_lock, I);
        }
    } else {
        while (1) {
            mcs_lock_acquire(mcs_lock, I);
            if (counter < n_increase) {
                counter++;
                increase++;
                mcs_lock_release(mcs_lock, I);
            } else {
                mcs_lock_release(mcs_lock, I);
                break;
            }
        }
    }
    
    pthreadbase_barrier_wait (barrier); // end racing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("mcs\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }
    
    mcs_perthread_increase[tid] = increase;
    increase = 0;
    
    pthreadbase_barrier_wait (barrier);

/* fai */
    if (tid == 0) {
        counter = 0;
        start_time = gethrtime_x86 ();
    }
    
    pthreadbase_barrier_wait (barrier); // start racing

    if (each) {
        for (unsigned long i = 0; i < n_increase; i++)
            fai (&counter);
    } else {
        while (1) {
            if (counter < n_increase) {
                fai (&counter);
                increase++;
            } else
                break;
        }
    }
    
    pthreadbase_barrier_wait (barrier); // end racing

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("fai\t%10.6f\t\t%lu\n", (finish_time-start_time)*1000, counter);
    }
    
    fai_perthread_increase[tid] = increase;
}









int main(int argc, char **argv) 
{
    // default global values
    n_thread = 4;
    n_increase = 10000;
    each = false;
    
    int c;
    while((c = getopt(argc, argv, "t:i:eh")) != -1) {
        switch(c) {
            case 't':
                n_thread = atoi(optarg);
                break;
            case 'i':
                n_increase = strtoul(optarg, NULL, 0);
                break;
            case 'e':
                each = true;
                break;
            case 'h':
                printf ("locks_test\n-t <number of threads> (default=4)\n-i <times to increase the counter> (default=10000)\n-e (each thread increase the counter i times, default=false)\n");
                return 0;
        }
    }

    printf ("\nthreads=%d\tincrease=%lu\t", n_thread, n_increase);
    if (each) printf ("total=false\n");
    else printf ("total=true\n");


    barrier = pthreadbase_barrier_init (n_thread);

    none_perthread_increase = new unsigned long[n_thread];
    tas_perthread_increase = new unsigned long[n_thread];
    tatas_perthread_increase = new unsigned long[n_thread];
    tatasbf_perthread_increase = new unsigned long[n_thread];
    ticket_perthread_increase = new unsigned long[n_thread];
    mcs_perthread_increase = new unsigned long[n_thread];
    fai_perthread_increase = new unsigned long[n_thread];

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
    
    if (!each) {
        printf ("\nHow many increases each thread contribute?\n");
        printf ("tid       none        tas      tatas    tatasbf     ticket        mcs        fai\n");
        for (int i = 0; i < n_thread; i++)
            printf ("%3d %10lu %10lu %10lu %10lu %10lu %10lu %10lu\n", i,
                    none_perthread_increase[i],
                    tas_perthread_increase[i],
                    tatas_perthread_increase[i],
                    tatasbf_perthread_increase[i],
                    ticket_perthread_increase[i],
                    mcs_perthread_increase[i],
                    fai_perthread_increase[i]);
    }


}
