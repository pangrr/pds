/* kmeans.h */


#ifndef _kmeans_h
#define _kmeans_h

#include "global.h"
#include "data.h"
#include "sort.h"

typedef struct
{
    DataSet* trainData;
    int nComp;
    int it;
    double eps;
    double** means;
    int* r;
} Kmeans;


void trainKmeans (Kmeans* kmeans);

void assign (Kmeans* kmeans);

int updateKMeans (Kmeans* kmeans);

int updateKMean (Kmeans* kmeans, int i);

double** randKMeans (Kmeans* kmeans, int nComp);

Kmeans* initKmeans (DataSet* trainData, int nComp, int it, double eps);

void printKMeans (Kmeans* kmeans);

void printKDiagCovs (Kmeans* kmeans);

double* getKDiagCov (Kmeans* kmeans, int i);

double** getKDiagCovs (Kmeans* kmeans);

double getDist (double* A, double* B, int n);

#endif
