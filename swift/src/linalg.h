/* linalg.h */

#ifndef _linalg_h
#define _linalg_h

#include <math.h>




/* Return the multiplication of two matrices. */
double* dot (double* A, double* B, int mA, int nA, int mB, int nB);

/* Return the inversion of a square matrix. */
double* inv (double* A, int n, double deter);

/* Return the determinant of a square matrix. */
double det (double* A, int n);

/* Compute determinant by first doing Gaussian Elimination with partial pivoting
 * then compute the product of the diagonal elements. */
double detGauss (double* A, int n);


/* Transpose a square matrix in place. */
void tran (double* A, int n);

/* Return the cofactor matrix of a square matrix. */
double* cof (double* A, int n);





/* Return a copy of give matrix. */
double* copyOut (double* A, int m, int n);

/* Copy elements from matrix B to matrix A. */
void copyIn (double* A, double* B, int m, int n);

/* Set all elements to given value. */
void set (double* A, int m, int n, double x);





/* Return a multiplication of a  matrix with a scala. */
double* multOut (double* A, int m, int n, double x);

/* In place multiplication of a matrix with a scala. */
void multIn (double* A, int m, int n, double x);


/* Return the subtraction of two matrices. */
double* subOut (double* A, double* B, int m, int n);

/* Add two matrices in place. */
void addIn (double* A, double* B, int m, int n);



/* Print matrix. */
void printArray (double* A, int m, int n);


#endif
