#include "barriers.h"


/////////////////////////////////////
// centralized sense-reversing barrier

cs_barrier_t* cs_barrier_init (int n_thread)
{
    cs_barrier_t *barrier = (cs_barrier_t*) malloc (sizeof (cs_barrier_t));
    
    barrier->arrived = 0;
    barrier->sense = true;
    barrier->n_thread = n_thread;

    return barrier;
}


void cs_barrier_wait (cs_barrier_t *barrier, bool *local_sense)
{
    *local_sense = !(*local_sense); // toggle local sense
    
    if (fai (&(barrier->arrived)) + 1 < (unsigned long)barrier->n_thread) {
        while (barrier->sense != *local_sense); // spin
    
    } else { // last arrive
        barrier->arrived = 0;
        barrier->sense = !(barrier->sense); // toggle global sense
    }
}







//////////////////////////////////////
// tournament barrier

tournament_barrier_t* tournament_barrier_init (int n_thread) 
{
    tournament_barrier_t *barrier = (tournament_barrier_t *) malloc (sizeof(tournament_barrier_t));
    
    barrier->opponent_default = false;

	barrier->n_round = ceil (log(n_thread)/log(2));


	// allocate array
    barrier->array = new round_t*[n_thread];
    
    for (int i = 0; i < n_thread; i++)
       (barrier->array)[i] = new round_t[barrier->n_round+1];

    round_t **array = barrier->array;    

    // initialize array
	for (int i = 0; i < n_thread; i++) {
		for (int j = 0; j <= barrier->n_round; j++) {
			
            array[i][j].flag = false;
			array[i][j].role = DEFAULT;
			array[i][j].opponent = &(barrier->opponent_default);
			
            // assign role and opponent
            int tmp = ceil (pow(2, j));
			int tmp2 = ceil (pow(2, j-1));

            if (j > 0 && i%tmp == 0 && i+tmp2 < n_thread && tmp < n_thread) {
				array[i][j].role = WINNER;
				array[i][j].opponent = &(array[i+tmp2][j].flag);
            
            } else if (j > 0 && i%tmp == 0 && i+tmp2 >= n_thread) {
				array[i][j].role = BYE;
            
            } else if (j > 0 && i%tmp == tmp2) {
				array[i][j].role = LOSER;
				array[i][j].opponent = &(array[i-tmp2][j].flag);
            
            } else if (j > 0 && i == 0 && tmp >= n_thread) {
			 	array[i][j].role = CHAMPION;
				array[i][j].opponent = &(array[i+tmp2][j].flag);
            
            } else if (j == 0) {
				array[i][j].role = DROPOUT;
            
            } else {
                array[i][j].role = DEFAULT;
            }
		}
	}

    return barrier;
}



void tournament_barrier_wait (tournament_barrier_t *barrier, int tid, bool *sense) 
{
    round_t **array = barrier->array;

	int round = 0;

	while (1) { // arrival

		if (array[tid][round].role == LOSER) {
			*((array[tid][round]).opponent) = *sense;
			while (array[tid][round].flag != *sense);
			break;
		
        } else if (array[tid][round].role == WINNER) {
			while (array[tid][round].flag != *sense);
		
        } else if (array[tid][round].role == CHAMPION) {
			while (array[tid][round].flag != *sense);
			*((array[tid][round]).opponent) = *sense;
			break;
        }
		
        if (round < barrier->n_round)
		    round++;
	}

	while(1) { // wake up
        if (round > 0)
		    round--;

		if (array[tid][round].role == WINNER) {
			*((array[tid][round]).opponent) = *sense;
        
        } else if (array[tid][round].role == DROPOUT) {
			break;
        }
	}

	*sense = !*sense;
}







///////////////////////////////////////////
// pthread-based condition synchronization
// Extracted from sor_pthread.c

pthreadbase_barrier_t *pthreadbase_barrier_init (int n_thread)
{
    pthreadbase_barrier_t *barrier = (pthreadbase_barrier_t *) malloc (sizeof(pthreadbase_barrier_t));
    
    barrier->n_thread = n_thread;
    barrier->arrived = 0;
    
    return barrier;
}


void pthreadbase_barrier_wait (pthreadbase_barrier_t *barrier)
{
    pthread_mutex_lock (&(barrier->mutex));	//lock

    (barrier->arrived)++;
    if (barrier->arrived < barrier->n_thread) {
        pthread_cond_wait (&(barrier->cond), &(barrier->mutex));
    }
    else {
        barrier->arrived = 0;		// reset the barrier before broadcast is important
        pthread_cond_broadcast (&(barrier->cond));
    }

    pthread_mutex_unlock (&(barrier->mutex));	//unlock
}

