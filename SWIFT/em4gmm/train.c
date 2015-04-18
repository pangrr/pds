/* train.c */

#include "global.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"


int main (int argc, char** argv)
{
    double begin, end;

    DataSet* dataSet = loadData ("train10x100k.dat", 10, 100000);

    GMM* gmm = initGMM (dataSet, 5, 20, 0, 0);
    
    begin = gethrtime_x86();
    trainEM (gmm, dataSet);
    end = gethrtime_x86();
    

    printf("%f sec\n", end-begin);
    
//    double A[9] = {6.0, 1.0, 1.0, 4.0, -2.0, 5.0, 2.0, 8.0, 7.0};
//    //printf ("%f\n",det(A, 3));
//    printf ("%f\n", detGauss(A, 3));



    return 0;
}
