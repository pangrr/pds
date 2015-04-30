/* train.c */

#include "global.h"
#include "kmeans.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"
#include "sort.h"

int main (int argc, char** argv)
{
    int nData = 100000;
    int sampleSize = 50000;
    int dim = 10;
    int nComp = 100;
    int fixRate = 10;
    double kmEps = 1e-6;
    double gmmEps = 1e-1;
    int gmmIt = 50;
    int kmIt = 1000;
    double begin, end;

    DataSet* trainData = loadData ("../data/10x100k", dim, nData);
    int* sample = randSample (nData, sampleSize);
    DataSet* sampleData = getSubDataSet (trainData, sample, sampleSize);
//    int i, j;
//    for (i = 0; i < sampleSize; i++)
//    {
//        for (j = 0; j < dim; j++)
//        {
//            printf ("%f ", sampleDataSet->dataSet[i][j]);
//        }
//        printf ("\n");
//    }
//    return 0;
    Kmeans* kmeans = initKmeans (sampleData, nComp, kmIt, kmEps);
    trainKmeans (kmeans);
    
    double** covs = getKDiagCovs (kmeans);
    double** means = kmeans->means;
    
    begin = gethrtime_x86();

    GMM* gmm = initGMM (trainData, sample, sampleSize, means, covs, nComp, fixRate, gmmIt, gmmEps);
    trainGMM (gmm);

    end = gethrtime_x86();
    printf("%f sec\n", end-begin);
    return 0;
  
    



//    int index[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//    double order[9] = {1.0, 5.0, 3.0, 1.0, 3.0, 2.0, 2.0, 4.0, -6.0};
//    double* inverse = inv (order, 3);
//    quickSort (index, order, 0, 8);
//    printDoubleArray (inverse, 3, 3);
//    printIntArray (index, 1, 9);
}
