/* kmeans.c */

#include "kmeans.h"


Kmeans* initKmeans (DataSet* trainData, int nComp, int it, double eps)
{
    Kmeans* kmeans = (Kmeans*) malloc (sizeof(Kmeans));
    kmeans->trainData = trainData;
    kmeans->nComp = nComp;
    kmeans->it = it;
    kmeans->eps = eps;
    kmeans->means = randKMeans (kmeans, nComp);
    kmeans->r = (int*) malloc (trainData->size*nComp*sizeof(int));
    return kmeans;
}



void trainKmeans (Kmeans* kmeans)
{
    int i, nConv;
//    int j, k;
    
    for (i = 0; i < kmeans->it; i++)
    {
        printf ("%d:", i);

        assign (kmeans);
        
//        for (j = 0; j < kmeans->trainData->size; j+=1000)
//        {
//            for (k = 0; k < kmeans->nComp; k++)
//            {
//                printf ("%d ", kmeans->r[j*kmeans->nComp+k]);
//            }
//            printf ("\n");
//        }
//        return;

        nConv = updateKMeans(kmeans);

//        for (j = 0; j < kmeans->nComp; j++)
//        {
//            for (k = 0; k < kmeans->trainData->dim; k++)
//            {
//                printf ("%f ", kmeans->means[j][k]);
//            }
//            printf ("\n");
//        }
//        return;

        printf ("%d ", nConv);
        fflush (stdout);
        if (nConv == kmeans->nComp)
        {
            break;
        }
    }
    printf ("\n");
//    printKMeans (kmeans);
//    printKDiagCovs (kmeans);
}




void assign (Kmeans* kmeans)
{
    int i, j;
    double minDist, dist;
    int nComp = kmeans->nComp;
    int* r = kmeans->r;
    double** means = kmeans->means;
    int trainSize = kmeans->trainData->size;
    double** dataSet = kmeans->trainData->dataSet;
    int dim = kmeans->trainData->dim;
    int clust;

//    #pragma omp parallel for private(i, clust, j, dist, minDist)
    for (i = 0; i < trainSize; i++)
    {
        clust = 0;
        for (j = 0; j < nComp; j++)
        {
            dist = getDist (means[j], dataSet[i], dim);
            if (j == 0)
            {
                minDist = dist;
            }
            else if (dist < minDist)
            {
                minDist = dist;
                clust = j;
            }
        }

        for (j = 0; j < nComp; j++)
        {
            if (j == clust)
            {
                r[i*nComp+j] = 1;
            }
            else
            {
                r[i*nComp+j] = 0;
            }
        }
    }
}



int updateKMean (Kmeans* kmeans, int i)
{
    int j, k, pop;
    int trainSize = kmeans->trainData->size;
    int nComp = kmeans->nComp;
    int dim = kmeans->trainData->dim;
    double** dataSet = kmeans->trainData->dataSet;
    int* r = kmeans->r;
    int isConv = 0;

    double* mean = (double*) malloc (dim*sizeof(double));
    pop = 0;
    
    for (j = 0; j < trainSize; j++)
    {
        if (r[j*nComp+i] == 1)
        {
            pop++;
            for (k = 0; k < dim; k++)
            {
                mean[k] += dataSet[j][k];
            }
        }
    }
    for (k = 0; k < dim; k++)
    {
        mean[k] /= pop;
    }
    double* oldMean = kmeans->means[i];
    kmeans->means[i] = mean;

    if (getDist(mean, oldMean, dim) < kmeans->eps)
    {
        isConv = 1;
    }

    free (oldMean);
    return isConv;
}







int updateKMeans (Kmeans* kmeans)
{
    int i;
    int nComp = kmeans->nComp;
    int nConv = 0;

    for (i = 0; i < nComp; i++)
    {
        nConv += updateKMean (kmeans, i);
    }

    return nConv;
}








double** getKDiagCovs (Kmeans* kmeans)
{
    int i;
    double** covs = (double**) malloc (kmeans->nComp*sizeof(double*));
    
//    #pragma omp parallel for private (i)
    for (i = 0; i < kmeans->nComp; i++)
    {
        covs[i] = getKDiagCov (kmeans, i);
    }
    return covs;
}







double* getKDiagCov (Kmeans* kmeans, int i)
{
    int j, k;
    int dim = kmeans->trainData->dim;
    int trainSize = kmeans->trainData->size;
    double** dataSet = kmeans->trainData->dataSet;
    int* r = kmeans->r;
    double sum, sub;
    double* cov = (double*) malloc (dim*sizeof(double));
    double* mean = kmeans->means[i];
    int pop = 0;

    for (j = 0; j < trainSize; j++)
    {
        if (r[j*kmeans->nComp+i] == 1)
        {
            pop++;
        }
    }

    int* member = (int*) malloc (pop*sizeof(int));

    k = 0;
    for (j = 0; j < trainSize; j++)
    {
        if (r[j*kmeans->nComp+i] == 1)
        {
            member[k] = j;
            k++;
        }
    }

    for (j = 0; j < dim; j++)
    {
        sum = 0.0;
        for (k = 0; k < pop; k++)
        {
            sub = dataSet[member[k]][j] - mean[j];
            sum += sub * sub;
        }
        cov[j] = sum / pop; 
    }

    free (member);
    return cov;
}







double** randKMeans (Kmeans* kmeans, int nComp)
{
    double** res = (double**) malloc (nComp*sizeof(double*));
    double** dataSet = kmeans->trainData->dataSet;
    int trainSize = kmeans->trainData->size;
    double* order = (double*) malloc (trainSize*sizeof(double));
    int* index = (int*) malloc (trainSize*sizeof(int));
    int dim = kmeans->trainData->dim;
    int i, j;
    double* mean;

    for (i = 0; i < trainSize; i++)
    {
        order[i] = (double)rand() / (double)RAND_MAX;
        index[i] = i;
    }

    quickSortDouble (index, order, 0, trainSize-1);

//    #pragma omp parallel for private(i, mean, j)
    for (i = 0; i < nComp; i++)
    {
        mean = (double*) malloc (dim*sizeof(double));
        for (j = 0; j < dim; j++)
        {
            mean[j] = dataSet[index[i]][j];
        }
        res[i] = mean;
    }
    return res;
}






double getDist (double* A, double* B, int n)
{
    int i;
    double res = 0.0;
    double sub;

    for (i = 0; i < n; i++)
    {
        sub = A[i] - B[i];
        res += sub * sub;
    }
    return res;
}


void printKDiagCovs (Kmeans* kmeans)
{
    int i, j;
    int nComp = kmeans->nComp;
    double* cov;
    double** covs = getKDiagCovs (kmeans);
    int dim = kmeans->trainData->dim;

    printf ("cov:\n");

    for (i = 0; i < nComp; i++)
    {
        cov = covs[i];
        for (j = 0; j < dim; j++)
        {
            printf ("%f ", cov[j]);
        }
        printf ("\n");
    }
}



void printKMeans (Kmeans* kmeans)
{
    int i, j;
    int nComp = kmeans->nComp;
    double* mean;
    int dim = kmeans->trainData->dim;

    printf ("mean:\n");

    for (i = 0; i < nComp; i++)
    {
        mean = kmeans->means[i];
        for (j = 0; j < dim; j++)
        {
            printf ("%f ", mean[j]);
        }
        printf ("\n");
    }
}

