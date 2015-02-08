#ifndef BARRIERS_H__
#define BARRIERS_H__

#define DEFAULT_NTHREADS 4
int nthreads = DEFAULT_NTHREADS;


/////////////////////////////////////
// centralized sense-reversing barrier

void csbarrier(int tid)
{
    static volatile unsigned long count = 0;
    static volatile unsigned int sense = 0;
    static volatile unsigned int thread_sense[MAX_THREADS] = {0};

    thread_sense[tid] = !thread_sense[tid]; // toggle local sense
    if (fai(&count) == nthreads-1) { // last thread
        count = 0; // reset count
        sense = !sense; // toggle global sense
    } else {
        while (sense != thread_sense[tid]); // spin
    }
}

///////////////////////////////////////////
// pthread-based condition synchronization
// Extracted from sor_pthread.c

//pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
void barrier (int expect)
{
    static int arrived = 0;

    pthread_mutex_lock (&mut);	//lock

    arrived++;
    if (arrived < expect)
        pthread_cond_wait (&cond, &mut);
    else {
        arrived = 0;		// reset the barrier before broadcast is important
        pthread_cond_broadcast (&cond);
    }

    pthread_mutex_unlock (&mut);	//unlock
}


#endif // BARRIERS_H__

