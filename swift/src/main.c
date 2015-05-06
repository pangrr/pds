/* train.c */

#include "global.h"
#include "kmeans.h"
#include "gmm.h"
#include "linalg.h"
#include "data.h"
#include "sort.h"
#include "search.h"

int main (int argc, char** argv)
{
    int nData = 100000;
    int sampleSize = 50000;
    int dim = 10;
    int nComp = 100;
    int fixRate = 10;
    double kmEps = 1e-4;
    double gmmEps = 1e-1;
    int gmmIt = 50;
    int kmIt = 1000;
    double begin, end;
    begin = gethrtime_x86();

    DataSet* trainData = loadData ("../data/10x100k", dim, nData);
    int* sample = randSample (nData, sampleSize);
    DataSet* sampleData = getSubDataSet (trainData, sample, sampleSize);
    
    Kmeans* kmeans = initKmeans (sampleData, nComp, kmIt, kmEps);
    trainKmeans (kmeans);
    
    double** covs = getKDiagCovs (kmeans);
    double** means = kmeans->means;
    
    GMM* gmm = initGMM (trainData, sample, sampleSize, means, covs, nComp, fixRate, gmmIt, gmmEps);
    trainGMM (gmm);

    end = gethrtime_x86();
    printf("%f sec\n", end-begin);
    return 0;
  
    



 //   int index[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//    printf ("%d\n", biSearch(3, index, 0, 8));
//    printf ("%d\n", biSearch(4, index, 0, 8));
//    printf ("%d\n", biSearch(10, index, 0, 8));
//    printf ("%d\n", biSearch(15, index, 0, 8));

//    int order[9] = {1, 5, 3, 1, 3, 2, 2, 4, -6};
//    quickSortInt (index, order, 0, 8);
//    int j;
//    for (j = 0; j < 9; j++)
//        printf ("%d ", index[j]);
}
