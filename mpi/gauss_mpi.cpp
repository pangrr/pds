// gaussian elimination
// mpi
// cyclic
// pipe-lining



#include <stdlib.h>
#include <iostream>

#include "mpi.h"

using namespace std;


int size,rank; 





void distribute_matrix (double *M, double *LM, int n)
{
    if (rank == 0) 
    {
        for (int p = size-1; p >= 0; p--) 
        {
            for (int i = p; i < n; i+=size)
                for (int j = 0; j < n; j++)
                    LM[(i/size)*n+j] = M[i*n+j];
            if (p != 0) 
                MPI_Send (LM, (n/size)*n, MPI_DOUBLE, p, 10, MPI_COMM_WORLD );
         }
    } 
    else
        MPI_Recv (LM, (n/size)*n, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}








void gather_matrix (double *M, double *LM, int n)
{
    if (rank == 0) 
    {
        for (int p = 0; p < size; p++) 
        {
            if (p != 0) 
                MPI_Recv (LM, (n/size)*n, MPI_DOUBLE, p, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = p; i < n; i+=size)
                for (int j = 0; j < n; j++)
                    M [i*n+j] = LM[(i/size)*n+j];
        }
    } else
        MPI_Send (LM, (n/size)*n, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD );
}








void parallel_gaussian (double *M, int n)
{

    // initialzie local matrix
    double *LM = new double[n*n/size];
    
    double *cur_row = new double[n];

    int np = (int) n/size;
    int pred, succ, start;

    pred = (size+(rank-1)) % size;
    succ = (rank+1) % size;

    
    distribute_matrix (M, LM, n);
  
    
    
    for (int k = 0; k < n; k++) 
    { 
        if ((k%size) == rank) 
        {
            // normalize the key row
            int ksn = (k/size)*n;
            for (int j = k+1; j < n; j++)
                LM[ksn+j] = LM[ksn+j] / LM[ksn+k];
            LM[ksn+k] = 1.0;

            for (int j = 0; j < n; j++)
                cur_row[j] = LM[(k/size)*n+j];


            // send row to successor
            MPI_Send (cur_row, n, MPI_DOUBLE, succ, 20, MPI_COMM_WORLD);
        } 
        else 
        {			
            // receive row from predecessor
            MPI_Recv (cur_row, n, MPI_DOUBLE, pred, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // forward row to successor
            if (succ != k%size)
                MPI_Send (cur_row, n, MPI_DOUBLE, succ, 20, MPI_COMM_WORLD);
        }




        start = (rank <= k%size) ? (int) k/size+1 : (int) k/size;	// compute starting point

        // eliminate rows
        for (int i = start; i < np; i++) 
        {
            int in = i*n; 
            int ink = in+k;
            for (int j = k+1; j < n; j++)
                LM[in+j] -= LM[ink] * cur_row[j];
            LM[ink] = 0.0;
        }
    } 
    
    
    gather_matrix (M, LM, n);		// gather matricies
} 



void printM (double* M, int n)
{
    for (int r = 0; r < n; r++)
    {
        for (int c = 0; c < n; c++)
            cout << M[r*n+c] << " ";

        cout << endl;
    }
}




int main ( int argc, char *argv[] )
{
    int n = 4000;
    double *M = new double[n*n];
    double stime, ftime;


    
    // initialize a matrix populated by random numbers
    for (int r = 0; r < n; r++) 
    { 
        for (int c = 0; c < n; c++) 
            M[r*n+c] = rand ();
    }

    MPI_Init (&argc,&argv);

    MPI_Comm_size (MPI_COMM_WORLD,&size);
    MPI_Comm_rank (MPI_COMM_WORLD,&rank);
    

    stime = MPI_Wtime ();


    parallel_gaussian (M, n);
       
    ftime = MPI_Wtime ();			
    MPI_Barrier (MPI_COMM_WORLD);

    if (rank == 0)
        cout << size << "\t" << ftime-stime << " s" << endl;

    return 0;
}
