/* train.c */

#include "global.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"


int main (int argc, char** argv)
{
    double begin, end;

    DataSet* trainData = loadData ("../data/SUB1_cct_gp.txt", 19, 30000);

    /* trainData, sampleSize, nComp, fixRate, iteration, eps */
    GMM* gmm = initGMM (trainData, 10000, 50, 10, 50, 1e-6);
    
    begin = gethrtime_x86();
    train (gmm);
    end = gethrtime_x86();
    

    printf("%f sec\n", end-begin);
    
//    double A[9] = {6.0, 1.0, 1.0, 4.0, -2.0, 5.0, 2.0, 8.0, 7.0};
//    //printf ("%f\n",det(A, 3));
//    printf ("%f\n", detGauss(A, 3));



    return 0;
}
