/* train.c */

#include "global.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"
#include "sort.h"

int main (int argc, char** argv)
{
//    double begin, end;
   DataSet* trainData = loadData ("../data/19x30k1", 19, 30000);
  
            /* trainData, sampleSize, nComp, fixRate, iteration, eps */
    GMM* gmm = initGMM (trainData, 30000, 50, 50, 50, 1e-10);
    
//    DataSet* trainData = loadData ("../data/19x30k1", 19, 30000);
//    GMM* gmm = initGMM (trainData, 10000, 10, 3, 20, 1e-10);

    
//    begin = gethrtime_x86();
    train (gmm);
//    end = gethrtime_x86();

//    printf("%f sec\n", end-begin);






//    int index[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//    double order[9] = {1.0, 5.0, 3.0, 1.0, 3.0, 2.0, 2.0, 4.0, -6.0};
//    double* inverse = inv (order, 3);
//    quickSort (index, order, 0, 8);
//    printDoubleArray (inverse, 3, 3);
//    printIntArray (index, 1, 9);


    return 0;
}
