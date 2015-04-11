/* train.c */

#include "global.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"


int main (int argc, char** argv)
{

    DataSet* dataSet = loadData ("train_2x1k.dat", 2, 1000);

    GMM* gmm = initGMM (dataSet, 3, 20, 0, 0);

    trainEM (gmm, dataSet);

/*

    double A[9] = {1.0, 2.0, 0.0, -1.0, 1.0, 1.0, 1.0, 2.0, 3.0};
    double B[4] = {0.728317, 0.841571, 0.841571, 0.972435};

    printArray (inv(B, 2), 2, 2);
    printf ("%.15f\n", det(B, 2));

*/


    return 0;
}
