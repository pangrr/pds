/* train.c */

#include "global.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"


int main (int argc, char** argv)
{
    double begin, end;

    DataSet* dataSet = loadData ("../data/train2x1k.dat", 2, 1000);

    GMM* gmm = initGMM (dataSet, 3, 20, 1e-5);
    
    begin = gethrtime_x86();
    trainEM (gmm);
    end = gethrtime_x86();
    

    printf("%f sec\n", end-begin);
    
//    double A[9] = {6.0, 1.0, 1.0, 4.0, -2.0, 5.0, 2.0, 8.0, 7.0};
//    //printf ("%f\n",det(A, 3));
//    printf ("%f\n", detGauss(A, 3));



    return 0;
}
