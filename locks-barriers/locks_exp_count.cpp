#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>


#include "locks.h"
#include "hrtimer_x86.h"

using namespace std;

unsigned long counter, n_increase, counter_sum;
double std_sum;
int max_n_thread, n_repeat, n_thread;
bool each;
pthread_barrier_t barrier;

double start_time, finish_time;

tatasbf_lock_t tatasbf_lock;
tatas_lock_t tatas_lock;
tas_lock_t tas_lock;
ticket_lock_t *ticket_lock;
mcs_qnode_t **mcs_lock;

int *perthread_increase;






void perthread_increase_clear ()
{
    for (int i = 0; i < n_thread; i++)
        perthread_increase[i] = 0;
}

double perthread_increase_sum ()
{
    double sum = 0.0;
    for (int i = 0; i < n_thread; i++)
        sum += perthread_increase[i];
    return sum;
}

double perthread_increase_mean ()
{
    return perthread_increase_sum ()/n_thread;
}

double perthread_increase_var ()
{
    double mean = perthread_increase_mean ();
    double sqr_sum = 0.0;
    for (int i = 0; i < n_thread; i++)
        sqr_sum += pow ((double)perthread_increase[i]-mean, 2);
    return sqr_sum / n_thread;
}

double perthread_increase_std ()
{
    return pow (perthread_increase_var (), 0.5);
}







void *thread_work (void *_tid) 
{
    int tid = *(int*)_tid;
    
    int increase; // how much does this thread contribute

    pthread_barrier_wait (&barrier); // all threads created

    
/*  no lock */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
        }
        
        pthread_barrier_wait (&barrier); // start racing
        

        if (each) {
            for (unsigned long i = 0; i < n_increase; i++)
                counter++;
        } else {
            while (counter < n_increase) {// who can beat me
                counter++;
                increase++;
            }
        }
        
        pthread_barrier_wait (&barrier); // end racing

        perthread_increase[tid] = increase;
        
        pthread_barrier_wait (&barrier);

        std_sum += perthread_increase_std ();

        if (tid == 0)
            counter_sum += counter;

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        if (each)
            printf ("%.2f,", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f,", std_sum/n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);


/* tas lock */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
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
                    increase++;
                } else {
                    tas_lock_release(&tas_lock);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end racing

        perthread_increase[tid] = increase;
        
        pthread_barrier_wait (&barrier);

        std_sum += perthread_increase_std ();
        
        if (tid == 0) 
            counter_sum += counter;

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        if (each)
            printf ("%.2f,", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f,", std_sum/n_repeat);
        fflush (stdout);
    }

        
    pthread_barrier_wait (&barrier);


/* tatas lock */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
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
                    increase++;
                } else {
                    tatas_lock_release(&tatas_lock);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end racing

        perthread_increase[tid] = increase;
        
        pthread_barrier_wait (&barrier);

        std_sum += perthread_increase_std ();

        if (tid == 0)
            counter_sum += counter;

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        if (each)
            printf ("%.2f,", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f,", std_sum/n_repeat);
        fflush (stdout);
    }
        
    pthread_barrier_wait (&barrier);

/* tatasbf lock */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
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
                    increase++;
                } else {
                    tatasbf_lock_release(&tatasbf_lock);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end reacing
        
        perthread_increase[tid] = increase;
        
        pthread_barrier_wait (&barrier);

        std_sum += perthread_increase_std ();
        
        if (tid == 0)
            counter_sum += counter;

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        if (each)
            printf ("%.2f,", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f,", std_sum/n_repeat);
        fflush (stdout);
    }

    pthread_barrier_wait (&barrier);

/* ticket lock */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
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
                    increase++;
                } else {
                    ticket_lock_release(ticket_lock);
                    break;
                }
            }
        }

        pthread_barrier_wait (&barrier); // end racing
        
        perthread_increase[tid] = increase;
        
        pthread_barrier_wait (&barrier);

        std_sum += perthread_increase_std ();
        
        if (tid == 0)
            counter_sum += counter;

        pthread_barrier_wait (&barrier);
    }

    if (tid == 0) {
        if (each)
            printf ("%.2f,", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f,", std_sum/n_repeat);
        fflush (stdout);
    }

    pthread_barrier_wait (&barrier);
    
    
/* mcs lock */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
        }
        
        mcs_qnode_t *I = mcs_qnode_init ();
        
        pthread_barrier_wait (&barrier); // start racing

        printf ("%d\n", k);
        
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
                    mcs_lock_release(mcs_lock, I);
                    increase++;
                } else {
                    mcs_lock_release(mcs_lock, I);
                    break;
                }
            }
        }
        
        pthread_barrier_wait (&barrier); // end racing

        perthread_increase[tid] = increase;
        pthread_barrier_wait (&barrier);

        if (tid == 0) {
            std_sum += perthread_increase_std ();
            counter_sum += counter;
        }

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        if (each)
            printf ("%.2f,", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f,", std_sum/n_repeat);
        fflush (stdout);
    }
    
    pthread_barrier_wait (&barrier);


/* fai */
    if (tid == 0) {
        counter_sum = 0;
        std_sum = 0.0;
    }
    
    
    for (int k = 0; k < n_repeat; k++) {
        if (tid == 0) {
            counter = 0;
            increase = 0;
        }
        
        pthread_barrier_wait (&barrier); // start racing

        if (each) {
            for (unsigned long i = 0; i < n_increase; i++)
                fai (&counter);
        } else
            while (fai (&counter) < n_increase - 1) {
                increase++;
            }
        
        pthread_barrier_wait (&barrier); // end racing

        perthread_increase[tid] = increase;
        
        pthread_barrier_wait (&barrier);
        
        if (tid == 0)
            counter_sum += counter;

        std_sum += perthread_increase_std ();

        pthread_barrier_wait (&barrier);
    }
    
    if (tid == 0) {
        if (each)
            printf ("%.2f\n", (double)counter_sum/n_repeat/n_thread);
        else
            printf ("%.2f\n", std_sum/n_repeat);
    }
        
}









int main(int argc, char **argv) 
{
    // default global values
    max_n_thread = 20;
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
                printf ("\nlocks_test\n-x <max number of threads> (default=20)\n-i <times to increase the counter> (default=10000)\n-e (each thread increase the counter i times, default=false)\n-r <repeat times> (default=5)\n\n");
                return 0;
        }
    }

    printf ("\nthreads=1~%d\tincrease=%lu\trepeat=%d\t", max_n_thread, n_increase, n_repeat);
    if (each) printf ("total=false\n");
    else printf ("total=true\n");

    printf ("none,tas,tatas,tatasbf,ticket,mcs,fai\n"); // head

    for (n_thread = 1; n_thread <= max_n_thread; n_thread++) {
        printf ("%d,", n_thread);

        // initialize
        pthread_barrier_init (&barrier, NULL, n_thread);

        int *tids = new int[n_thread];
        pthread_t *threads = new pthread_t[n_thread];
        
        delete[] perthread_increase;
        perthread_increase = new int[n_thread];
        
        tas_lock_init (&tas_lock);
        tatas_lock_init (&tatas_lock);
        tatasbf_lock_init (&tatasbf_lock);
        delete ticket_lock;
        ticket_lock = ticket_lock_init ();
        delete mcs_lock;
        mcs_lock = mcs_lock_init ();
            

        // launch threads
        for (int i = 0; i < n_thread; i++) {
            tids[i] = i;
            pthread_create (&threads[i], NULL, thread_work, (void*)&tids[i]);
        }
        

        for (int i = 0; i < n_thread; i++)
            pthread_join(threads[i], NULL);
    }

    printf ("\n");
}
