#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "barriers.h"
#include "hrtimer_x86.h"

using namespace std;

int max_n_thread, n_repeat;
unsigned long n_barrier, distance;

double start_time, finish_time, accumulate_time;

tournament_barrier_t *tournament_barrier;
cs_barrier_t *cs_barrier;
pthreadbase_barrier_t *pthreadbase_barrier;
pthread_barrier_t barrier;


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

    pthread_barrier_wait (&barrier); // all threads created

/* pthreadbase barrier */
    if (tid == 0)
        accumulate_time = 0.0;

    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0)
            start_time = gethrtime_x86 ();

        pthread_barrier_wait (&barrier);

        for (unsigned long i = 0; i < n_barrier; i++) {
            critical_section (distance);
            pthreadbase_barrier_wait (pthreadbase_barrier);
        }

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


/* cs barrier */
    if (tid == 0)
        accumulate_time = 0.0;

    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0)
            start_time = gethrtime_x86 ();

        bool sense = true;
        
        pthread_barrier_wait (&barrier);
     
        for (unsigned long i = 0; i < n_barrier; i++) {
            critical_section (distance);
            cs_barrier_wait (cs_barrier, &sense);
        }
        

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



/* tournament barrier */
    if (tid == 0)
        accumulate_time = 0.0;

    for (int i = 0; i < n_repeat; i++) {
        if (tid == 0)
            start_time = gethrtime_x86 ();

        bool sense = true;

        pthread_barrier_wait (&barrier);

        for (unsigned long i = 0; i < n_barrier; i++) {
            critical_section (distance);
            tournament_barrier_wait (tournament_barrier, tid, &sense);
        }
        
        if (tid == 0) {
            finish_time = gethrtime_x86 ();
            accumulate_time += finish_time - start_time;
        }
        pthread_barrier_wait (&barrier);
    }

    if (tid == 0)
        printf ("%f\n", accumulate_time / n_repeat);
}






int main(int argc, char **argv) 
{
    // default values of global variables
    max_n_thread = 20;
    n_barrier = 10000;
    distance = 10;
    n_repeat = 5;

    int c;
    while ((c = getopt (argc, argv, "x:i:d:h")) != -1) {
        switch (c) {
            case 'r':
                n_repeat = atoi (optarg);
                break;
            case 'x':
                max_n_thread = atoi (optarg);
                break;
            case 'i':
                n_barrier = strtoul (optarg, NULL, 0);
                break;
            case 'd':
                distance = strtoul (optarg, NULL, 0);
                break;
            case 'h':
                printf("\nbarriers_test\n-x <max number of threads> (default=20)\n-i <number of barriers> (default=10000)\n-d <distance between barriers> (default=10)\n-r <number of repeats> (default=5)\n\n");
                return 0;
        }
    }

    printf ("\nthreads=2~%d\tbarriers=%lu\tdistance=%lu\n\n", max_n_thread, n_barrier, distance);
    printf ("pthreadbase,cs,tournament\n");

    // launch threads
    for (int n_thread = 2; n_thread <= max_n_thread; n_thread++) {
        printf ("%d,", n_thread);
        
        pthread_t *threads = new pthread_t[n_thread];
        
        int *tids = new int[n_thread];
        for (int i = 0; i < n_thread; i++)
            tids[i] = i;
        
        delete tournament_barrier;
        tournament_barrier = tournament_barrier_init (n_thread);
        delete pthreadbase_barrier;
        pthreadbase_barrier = pthreadbase_barrier_init (n_thread);
        delete cs_barrier;
        cs_barrier = cs_barrier_init (n_thread);
        pthread_barrier_init (&barrier, NULL, n_thread);
            
        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, thread_work, (void*)&tids[i]);   
        

        for (int i = 0; i < n_thread; i++)
            pthread_join(threads[i], NULL);
    }

    printf ("\n");
}
