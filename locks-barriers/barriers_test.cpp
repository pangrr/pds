#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barriers.h"
#include "hrtimer_x86.h"

using namespace std;

int n_thread;
unsigned long n_barrier, distance;

double start_time, finish_time;

tournament_barrier_t *tournament_barrier;
cs_barrier_t *cs_barrier;
pthreadbase_barrier_t *pthreadbase_barrier;


/* test barriers's performance on critical sections of different length */
void critical_section (unsigned long length)
{
    for (unsigned long i = 0; i < length; i++) {
        double d = 123456789 / 987654321;
    }
}


void* thread_work (void* _tid)
{
    int tid = *(int*)_tid;

    pthreadbase_barrier_wait (pthreadbase_barrier); // all threads created

/* pthreadbase barrier */
    if (tid == 0)
        start_time = gethrtime_x86 ();

    pthreadbase_barrier_wait (pthreadbase_barrier);

    for (unsigned long i = 0; i < n_barrier; i++) {
        critical_section (distance);
        pthreadbase_barrier_wait (pthreadbase_barrier);
    }

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("\t\t\t\tmilliseconds\n");
        printf ("pthread based\t\t\t%10.6f\n", (finish_time-start_time)*1000);
    }

    pthreadbase_barrier_wait (pthreadbase_barrier);

/* cs barrier */
    if (tid == 0)
        start_time = gethrtime_x86 ();

    bool sense = true;
    
    pthreadbase_barrier_wait (pthreadbase_barrier);
 
    for (unsigned long i = 0; i < n_barrier; i++) {
        critical_section (distance);
        cs_barrier_wait (cs_barrier, &sense);
    }
    

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("centralized sense-reverse\t%10.6f\n", (finish_time-start_time)*1000);
    }
    
    pthreadbase_barrier_wait (pthreadbase_barrier);

/* tournament barrier */
    if (tid == 0)
        start_time = gethrtime_x86 ();

    sense = true;

    pthreadbase_barrier_wait (pthreadbase_barrier);

    for (unsigned long i = 0; i < n_barrier; i++) {
        critical_section (distance);
        tournament_barrier_wait (tournament_barrier, tid, &sense);
    }
    
    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tournament\t\t\t%10.6f\n", (finish_time-start_time)*1000);
    }
}





int main(int argc, char **argv) 
{
    // default values of global variables
    n_thread = 4;
    n_barrier = 10000;
    distance = 10;

    int c;
    while ((c = getopt (argc, argv, "t:i:d:h")) != -1) {
        switch (c) {
            case 't':
                n_thread = atoi (optarg);
                break;
            case 'i':
                n_barrier = strtoul (optarg, NULL, 0);
                break;
            case 'd':
                distance = strtoul (optarg, NULL, 0);
                break;
            case 'h':
                printf("barriers_test\n-t <number of threads> (default=4)\n-i <number of barriers> (default=10000)\n-d <distance between barriers> (default=10)\n");
                return 0;
        }
    }

    printf ("\nthreads=%d\tbarriers=%lu\tdistance=%lu\n\n", n_thread, n_barrier, distance);

    // launch threads
    pthread_t *threads = new pthread_t[n_thread];
    
    int *tids = new int[n_thread];
    for (int i = 0; i < n_thread; i++)
        tids[i] = i;
    
    tournament_barrier = tournament_barrier_init (n_thread);
    pthreadbase_barrier = pthreadbase_barrier_init ( n_thread);
    cs_barrier = cs_barrier_init (n_thread);
        
    for (int i = 0; i < n_thread; i++)
        pthread_create (&threads[i], NULL, thread_work, (void*)&tids[i]);   
    

    for (int i = 0; i < n_thread; i++)
        pthread_join(threads[i], NULL);
}
