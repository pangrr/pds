// gaussian elimination
// cyclic
// pthread


#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <iostream>

#include "hrtimer_x86.h"

using namespace std;

// global variables
pthread_barrier_t barrier;



double **M;
int n;
int nthread;






void row_norm (int r, int start_c) 
{
    for (int c = start_c+1; c < n; c++)
		M[r][c] /= M[r][start_c];
    
    M[r][start_c] = 1.0;
}






void row_eliminate (int key_row, int thread_id) 
{

    for (int r = key_row + thread_id; r < n; r += nthread) 
    {
        if (r > key_row) {

            for (int c = key_row+1; c < n; c++)
                M[r][c] -= M[r][key_row] * M[key_row][c];
            
            M[r][key_row] = 0.0;
        }
    }
}





void *work_thread (void *lp) 
{
	
    // initialize
	int thread_id = *((int *)lp);
	pthread_barrier_wait(&barrier);

	// start gaussian elimination
	for (int key_row = 0; key_row < n; key_row++) 
    { 
        // normalize the key row
        if (thread_id == 0)
            row_norm (key_row, key_row);


        pthread_barrier_wait (&barrier);

         
        
        // elimination
        row_eliminate (key_row, thread_id);
        
        pthread_barrier_wait (&barrier);

    }
}




void printM ()
{
    for (int r = 0; r < n; r++)
    {
        for (int c = 0; c < n; c++)
            cout << M[r][c] << " ";
        
        cout << endl;
    }
    
}





int main (int argc, char *argv[]) {
    
    nthread = 1;
	pthread_attr_t attr;
	pthread_t *tid;
	int *id;

	// input arguments
	int c;
	while ((c = getopt (argc, argv, "p:")) != -1) 
    {
		switch (c) 
        {
	        case 'p':
			    nthread = atoi (optarg);
			    break;
		}
	}
    
    // initialize matrix
    n = 4000;
    M = new double*[n];
    for (int r = 0; r < n; r++)
        M[r] = new double[n];
    
    for (int r = 0; r < n; r++)
        for (int c = 0; c < n; c++)
            M[r][c] = rand ();





    double stime = gethrtime_x86();

    // initialize pthread
    pthread_barrier_init (&barrier, NULL, nthread);
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

    // launch threads
    id = new int[nthread]; 
    tid = new pthread_t[nthread];
    pthread_attr_init (&attr);
    
    for (int i = 0; i < nthread; i++) 
    {
        id[i] = i;
        pthread_create (&tid[i], &attr, work_thread, (void *)&id[i]);
    }

    // join threads
    for (int i = 0; i < nthread; i++)
        pthread_join (tid[i], NULL);



        
    double ftime = gethrtime_x86 ();


    cout <<  nthread << "\t" << ftime-stime << " s" << endl;
	return 0;
}

