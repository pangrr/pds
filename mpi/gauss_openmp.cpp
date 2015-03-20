// gauss elimination
// openmp

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include "hrtimer_x86.h"

using namespace std;



void printM (double** M, int n)
{
    for (int r = 0; r < n; r++)
    {
        for (int c = 0; c < n; c++)
            cout << M[r][c] << " ";

        cout << endl;
    }
}




int main (int argc, char** argv)
{
    double** M;
    int nthread_limit = 1;
    int n;
    int nthread;

    // parse input arguments
    int c;
    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                nthread_limit = atoi (optarg);
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

    double stime = gethrtime_x86 ();

    
    
    // gaussian elimination
    for (int key_row = 0; key_row < n; key_row++) 
    {
        // division on the key row
        for(int c = key_row+1; c < n; c++)
            M[key_row][c] /= M[key_row][key_row];
        
        M[key_row][key_row] = 1.0;


        // row elimination
        omp_set_num_threads (nthread_limit);
        #pragma omp parallel for
        
        for (int r = key_row+1; r < n; r++) 
        {
            nthread = omp_get_num_threads ();
            
            for (int c = key_row+1; c < n; c++)
                M[r][c] -= M[r][key_row] * M[key_row][c];
            
            M[r][key_row] = 0.0;
        }
    }

    double ftime = gethrtime_x86 ();


    cout << nthread << "\t" << ftime - stime << " s" << endl;
    return 0;
}
