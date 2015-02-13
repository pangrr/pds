#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barriers.h"
#include "hrtimer_x86.h"

using namespace std;

int n_thread, n_barrier, cs_len;

double start_time, finish_time;

tournament_barrier_t *tournament_barrier;
cs_barrier_t *cs_barrier;
pthreadbase_barrier_t *pthreadbase_barrier;


/* test barriers's performance on critical sections of different length */
void critical_section (int length)
{
    double d;
    for (int i = 0; i < length; i++) {
        d = 123456789 / 987654321;
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

    for (int i = 0; i < n_barrier; i++) {
        critical_section (cs_len);
        pthreadbase_barrier_wait (pthreadbase_barrier);
    }

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("\t\t\t\tmilliseconds\n");
        printf ("pthread based\t\t\t%f\n", (finish_time-start_time)*1000);
    }

    pthreadbase_barrier_wait (pthreadbase_barrier);

/* cs barrier */
    if (tid == 0)
        start_time = gethrtime_x86 ();

    bool sense = true;
    
    pthreadbase_barrier_wait (pthreadbase_barrier);
 
    for (int i = 0; i < n_barrier; i++) {
        critical_section (cs_len);
        cs_barrier_wait (cs_barrier, &sense);
    }
    

    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("centralized sense-reverse\t%f\n", (finish_time-start_time)*1000);
    }
    
    pthreadbase_barrier_wait (pthreadbase_barrier);

/* tournament barrier */
    if (tid == 0)
        start_time = gethrtime_x86 ();

    sense = true;

    pthreadbase_barrier_wait (pthreadbase_barrier);

    for (int i = 0; i < n_barrier; i++) {
        critical_section (cs_len);
        tournament_barrier_wait (tournament_barrier, tid, &sense);
    }
    
    if (tid == 0) {
        finish_time = gethrtime_x86 ();
        printf ("tournament\t\t\t%f\n", (finish_time-start_time)*1000);
    }
}





int main(int argc, char **argv) 
{
    // default values of global variables
    n_thread = 10;
    n_barrier = 100;
    cs_len = 10;

    int c;
    while ((c = getopt (argc, argv, "t:i:c:h")) != -1) {
        switch (c) {
            case 't':
                n_thread = atoi (optarg);
                break;
            case 'i':
                n_barrier = atoi (optarg);
                break;
            case 'c':
                cs_len = atoi (optarg);
                break;
            case 'h':
                printf("barriers_test\n-t <number of threads> (default=10)\n-i <number of barriers> (default=100)\n-c <relative length of execution time in between barriers> (default=10)\n");
                return 0;
        }
    }


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
