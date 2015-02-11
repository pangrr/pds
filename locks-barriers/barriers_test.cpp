#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>

#include "barriers.h"
#include "hrtimer_x86.h"

using namespace std;

int n_thread, n_barrier;

tournament_barrier_t *tournament_barrier;
cs_barrier_t *cs_barrier;
pthreadbase_barrier_t *pthreadbase_barrier;





void *tournament_barrier_thread (void *_tid)
{
    int tid = *(int*)_tid;
    bool sense = true;

    for (int i = 0; i < n_barrier; i++)
        tournament_barrier_wait (tournament_barrier, tid, &sense);
}


void *cs_barrier_thread (void *argv)
{
    bool local_sense = true;
    
    for (int i = 0; i < n_barrier; i++)
        cs_barrier_wait (cs_barrier, &local_sense);
}



void *pthreadbase_barrier_thread (void * argv)
{
    for (int i = 0; i < n_barrier; i++)
        pthreadbase_barrier_wait (pthreadbase_barrier);
}




int main(int argc, char **argv) 
{
    char *_barrier_type;

    int c;
    while ((c = getopt (argc, argv, "b:t:i:")) != -1) {
        switch (c) {
            case 't':
                n_thread = atoi (optarg);
                break;
            case 'b':
                _barrier_type = optarg;
                break;
            case 'i':
                n_barrier = atoi (optarg);
                break;
        }
    }

    string barrier_type (_barrier_type);
    

    // launch threads
    pthread_t *threads = (pthread_t *) malloc (sizeof(pthread_t) * n_thread);

    double start_time = gethrtime_x86 ();

    if (barrier_type.compare (PTHREAD) == 0) {
        pthreadbase_barrier = pthreadbase_barrier_init ( n_thread);
        
        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, pthreadbase_barrier_thread, NULL);
    
    
    
    } else if (barrier_type.compare (CS) == 0) {
        cs_barrier = cs_barrier_init (n_thread);

        for (int i = 0; i < n_thread; i++)
            pthread_create (&threads[i], NULL, cs_barrier_thread, NULL);
    
    
    
    } else if (barrier_type.compare (TOURNAMENT) == 0) {
        tournament_barrier = tournament_barrier_init (n_thread);
        int *tid = new int[n_thread];
        
        for (int i = 0; i < n_thread; i++) {
            tid[i] = i;

            pthread_create (&threads[i], NULL, tournament_barrier_thread, (void*)&tid[i]);   
        }

    
    
    } else {
        printf("Barrier types include: \n%s \n%s \n%s\n", PTHREAD, CS, TOURNAMENT);
        exit(-1);
    }

    for (int i = 0; i < n_thread; i++)
        pthread_join(threads[i], NULL);

    double finish_time = gethrtime_x86 ();

    printf ("%f milliseconds\n", (finish_time-start_time) * 1000);
}
