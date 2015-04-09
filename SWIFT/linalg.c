/* linalg.c */


#include "linalg.h"
#include "global.h"





double* dot (double* A, double* B, int mA, int nA, int mB, int nB)
{
    /* Check matrix size. */
    if (nA != mB)
    {
        printf ("Unmathing matrix size!");
    }

    int m = mA;
    int n = nB;
    int l = nA;

    double* C = (double*) malloc (m*n*sizeof(double));

    int i, j, k;
    double sum;

    /* Multiplication. */
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            sum = 0.0;

            for (k = 0; k < l; k++)
            {
                sum += A[i*l+k] * B[k*n+j];
            }

            C[i*n+j] = sum;
        }
    }

    return C;
}





double* inv (double* A, int n)
{
    double* C = cof (A, n);
    tran (C, n);
    multIn (C, n, n, 1.0/det(A, n));
    return C;
}













double det (double* A, int n)
{
    int i, j, j1, j2;
    double deter = 0.0;
    double* M = NULL;

    if (n < 1)
    {
        printf ("Matrix size less than 1!\n");
        return 0.0;
    }
    else if (n == 1)
    {
        deter = A[0];
    }
    else if (n == 2)
    {
        deter = A[0] * A[3] - A[2] * A[1];
    }
    else
    {
        for (j1 = 0; j1 < n; j1++)
        {
            M = (double*) malloc ((n-1)*(n-1)*sizeof(double));
            for (i = 1; i < n; i++)
            {
                j2 = 0;
                for (j = 0; j < n; j++)
                {
                    if (j == j1) continue;
                    M[(i-1)*(n-1)+j2] = A[i*n+j];
                    j2++;
                }
            }
            deter += pow (-1.0, j1+2.0) * A[j1] * det (M, n-1);
            free (M);
        }
    }

    return deter;
}







void tran (double* A, int n)
{
    int i, j;
    double tmp;

    for (i = 1; i < n; i++)
    {
        for (j = 0; j < i; j++)
        {
            tmp = A[i*n+j];
            A[i*n+j] = A[j*n+i];
            A[j*n+i] = tmp;
        }
    }
}








double* cof (double* A, int n)
{
    int i, j, ii, jj, i1, j1;
    double deter;
    double* C = (double*) malloc (n*n*sizeof(double));
    double* M = (double*) malloc ((n-1)*(n-1)*sizeof(double));

    for (j = 0; j < n; j++)
    {
        for (i = 0; i < n; i++)
        {
            i1 = 0;
            for (ii = 0; ii < n; ii++)
            {
                if (ii == i) continue;
                j1 = 0;
                for (jj = 0; jj < n; jj++)
                {
                    if (jj == j) continue;
                    M[i1*(n-1)+j1] = A[ii*n+jj];
                    j1++;
                }
                i1++;
            }

            deter = det (M, n-1);

            C[i*n+j] = pow (-1.0, i+j+2.0) * deter;
        }
    }

    free (M);
    return C;
}



double* copyOut (double* A, int m, int n)
{
    double* C = (double*) malloc (m*n*sizeof(double));
    
    int i, j;
    
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            C[i*n+j] = A[i*n+j];
        }
    }

    return C;
}




void copyIn (double* A, double* B, int m, int n)
{
    int i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            A[i*n+j] = B[i*n+j];
        }
    }
}


void set (double* A, int m, int n, double x)
{
    int i, j;
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            A[i*n+j] = x;
        }
    }
}

double* subOut (double* A, double* B, int m, int n)
{
    double* C = (double*) malloc (m*n*sizeof(double));

    int i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
           C[i*n+j] = A[i*n+j] - B[i*n+j];
        }
    }

    return C;
}


void addIn (double* A, double* B, int m, int n)
{
    int i, j;
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            A[i*n+j] += B[i*n+j];
        }
    }

}




double* multOut (double* A, int m, int n , double x)
{
    double* C = (double*) malloc (m*n*sizeof(double));
    int i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            C[i*n+j] = A[i*n+j] * x;
        }
    }
    return C;
}


void multIn (double* A, int m, int n , double x)
{
    int i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            A[i*n+j] *= x;
        }
    }
}




void printArray (double* A, int m, int n)
{
    int i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf ("%f ", A[i*n+j]);
        }
        printf ("\n");
    }
}
