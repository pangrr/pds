#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#include "locks.h"
#include "barriers.h" // ensure threads start counting at nearly the same time
#include "hrtimer_x86.h"

using namespace std;

unsigned long counter, n_increase;
int max_n_thread, n_repeat;
bool each;
pthread_barrier_t barrier;

double start_time, finish_time, accumulate_time;

tatasbf_lock_t tatasbf_lock;
tatas_lock_t tatas_lock;
tas_lock_t tas_lock;
ticket_lock_t *ticket_lock;
mcs_qnode_t **mcs_lock;



void *thread_work (void *_tid) 
{
    int tid = *(int*)_tid;
    
    int increase = 0; // how much does this thread contribute

    pthread_barrier_wait (&barrier); // all threads created

    
/*  no lock */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        pthread_barrier_wait (&barrier); // start racing
        

        if (each) {
            for (unsigned long i = 0; i < n_increase; i++)
                counter++;
        } else {
            while (counter < n_increase) // who can beat me
                counter++;
        }
        
        pthread_barrier_wait (&barrier); // end racing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        printf ("%f,", accumulate_time / n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);


/* tas lock */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        pthread_barrier_wait (&barrier); // start racing

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
                    tas_lock_release(&tas_lock);
                } else {
                    tas_lock_release(&tas_lock);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end racing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }
        
        
        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        printf ("%f,", accumulate_time / n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);


/* tatas lock */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        pthread_barrier_wait (&barrier); // start racing

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
                    tatas_lock_release(&tatas_lock);
                } else {
                    tatas_lock_release(&tatas_lock);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end racing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }
        
        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        printf ("%f,", accumulate_time / n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);

/* tatasbf lock */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        pthread_barrier_wait (&barrier); // start racing

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
                    tatasbf_lock_release(&tatasbf_lock);
                } else {
                    tatasbf_lock_release(&tatasbf_lock);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end reacing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        printf ("%f,", accumulate_time / n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);

/* ticket lock */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        pthread_barrier_wait (&barrier); // start racing

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
                    ticket_lock_release(ticket_lock);
                } else {
                    ticket_lock_release(ticket_lock);
                    break;
                }
            }
        }

        pthread_barrier_wait (&barrier); // end racing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }
        
        pthread_barrier_wait (&barrier);
    }
     
    if (tid == 0) {
        printf ("%f,", accumulate_time / n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);

    
/* mcs lock */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        mcs_qnode_t *I = mcs_qnode_init ();
        
        pthread_barrier_wait (&barrier); // start racing

        
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
        
        pthread_barrier_wait (&barrier); // end racing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }
        
        pthread_barrier_wait (&barrier);
    }

    if (tid == 0) {
        printf ("%f,", accumulate_time / n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);


/* fai */
    if (tid == 0)
        accumulate_time = 0.0;
    
    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0) {
            counter = 0;
            start_time = gethrtime_x86 ();
        }
        
        pthread_barrier_wait (&barrier); // start racing

        if (each) {
            for (unsigned long i = 0; i < n_increase; i++)
                fai (&counter);
        } else
            while (fai (&counter) < n_increase - 1);
        
        pthread_barrier_wait (&barrier); // end racing

        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }
    }
    
    if (tid == 0)
        printf ("%f\n", accumulate_time / n_repeat);
        
}









int main(int argc, char **argv) 
{
    // default global values
    max_n_thread = 30;
    n_repeat = 5;
    n_increase = 10000;
    each = false;
    
    int c;
    while((c = getopt(argc, argv, "r:x:i:eh")) != -1) {
        switch(c) {
            case 'r':
                n_repeat = atoi(optarg);
                break;
            case 'x':
                max_n_thread = atoi(optarg);
                break;
            case 'i':
                n_increase = strtoul(optarg, NULL, 0);
                break;
            case 'e':
                each = true;
                break;
            case 'h':
                printf ("\nlocks_test\n-x <max number of threads> (default=30)\n-i <times to increase the counter> (default=10000)\n-e (each thread increase the counter i times, default=false)\n-r <repeat times> (default=5)\n\n");
                return 0;
        }
    }

    printf ("\nthreads=1~%d\tincrease=%lu\trepeat=%d\t", max_n_thread, n_increase, n_repeat);
    if (each) printf ("total=false\n");
    else printf ("total=true\n");

    printf ("none,tas,tatas,tatasbf,ticket,mcs,fai\n"); // head

    for (int n_thread = 1; n_thread <= max_n_thread; n_thread++) {
        printf ("%d,", n_thread);

        pthread_barrier_init (&barrier, NULL, n_thread);


        // launch threads

        int *tids = new int[n_thread];
        pthread_t *threads = new pthread_t[n_thread];
        
        tas_lock_init (&tas_lock);
        tatas_lock_init (&tatas_lock);
        tatasbf_lock_init (&tatasbf_lock);
        delete ticket_lock;
        ticket_lock = ticket_lock_init ();
        delete mcs_lock;
        mcs_lock = mcs_lock_init ();
            

        for (int i = 0; i < n_thread; i++) {
            tids[i] = i;
            pthread_create (&threads[i], NULL, thread_work, (void*)&tids[i]);
        }
        

        for (int i = 0; i < n_thread; i++)
            pthread_join(threads[i], NULL);
    }
    printf ("\n");
}
