/* gmm.c */

#include "gmm.h"
#include "linalg.h"
#include "global.h"
#include "data.h"





GMM* initGMM (DataSet* trainData, int nComp, int it, double eps)
{
    int i;
    int dim = trainData->dim;
    int dataSize = trainData->size;
    double* cov = initCov (trainData);
    double* mean;
    double mix = 1.0 / nComp;

    GMM* gmm = (GMM*) malloc (sizeof(GMM));
    gmm->trainData = trainData;
    gmm->comp = (Gauss**) malloc (nComp*sizeof(Gauss*));
    gmm->dim = dim;
    gmm->nComp = nComp;
    gmm->it = it;
    gmm->eps = eps;
    gmm->resp = (double*) malloc (dataSize*nComp*sizeof(double));
    gmm->like = (double*) malloc (it*sizeof(double));

    /* Initialize components. */
    for (i = 0; i < nComp; i++)
    {
        mean = randData (trainData);
        gmm->comp[i] = initGauss (dim, mean, cov, mix);
    }
    
    return gmm;
}








double* randData (DataSet* dataSet)
{
    int dataSize = dataSet->size;
    int r = rand () % dataSize;
    return dataSet->dataSet[r];
}





double* initCov (DataSet* dataSet)
{
    int dataSize = dataSet->size;
    int dim = dataSet->dim;
    double* cov = (double*) malloc (dim*dim*sizeof(double));
    double* mean = (double*) malloc (dim*sizeof(double));
    int i;


    /* Compute mean of all data points. */
    set (mean, 1, dim, 0.0);
    for (i = 0; i < dataSize; i++)
    {
        addIn (mean, dataSet->dataSet[i], 1, dim);
    }
    multIn (mean, 1, dim, 1.0/dataSize);


    /* Compute covariance matrix of all data points. */
    set (cov, dim, dim, 0.0);
    for (i = 0; i < dataSize; i++)
    {
        double* sub = subOut (dataSet->dataSet[i], mean, 1, dim);
        double* mult = dot (sub, sub, dim, 1, 1, dim);
        addIn (cov, mult, dim, dim);
        
        free (sub);
        free (mult);
    }
    multIn (cov, dim, dim, 1.0/dataSize);

    free (mean);
    return cov;
}






Gauss* initGauss (int dim, double* mean, double* cov, double mix)
{
    Gauss* gauss = (Gauss*) malloc (sizeof(Gauss));

    gauss->mix = mix;
    gauss->pop = 0.0;
    gauss->dim = dim;

    gauss->mean = copyOut (mean, 1, dim);
    gauss->cov = copyOut (cov, dim, dim);
    
    gauss->covDet = detGauss (gauss->cov, dim);
    gauss->covInv = inv (gauss->cov, dim, gauss->covDet);
    gauss->probCoef = 1.0 / pow (pow(2*M_PI, dim)*gauss->covDet, 0.5);
    
    return gauss;
}





int trainEM (GMM* gmm)
{
    int i;
    int it = gmm->it;

    for (i = 0; i < it; i++)
    {
        EStep (gmm);
        MStep (gmm);
        like (gmm, gmm->trainData, i);
        printf ("Log likelihood: %f\n", gmm->like[i]);
        if (isConverge(gmm, i) == 1)
            return 1;
    }
    return 0;
}





int isConverge (GMM* gmm, int it)
{
    if (fabs((gmm->like[it]-gmm->like[it-1])/gmm->like[it-1]) < gmm->eps)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}





double gaussProb (Gauss* gauss, double* dataPoint)
{
    int dim = gauss->dim;
    double* mean = gauss->mean;

    /* Temporary memory to be freed. */
    double* sub = subOut (dataPoint, mean, 1, dim);
    double* partA = dot (dot(sub, gauss->covInv, 1, dim, dim, dim), sub, 1, dim, dim, 1);
    double prob = gauss->probCoef * exp(-(*partA)/2.0);

    free (sub);
    free (partA);
    return prob;
}





void like (GMM* gmm, DataSet* dataSet, int it)
{
    int dataSize = dataSet->size;
    int nComp = gmm->nComp;

    double sumIn;
    Gauss* gauss;
    double* dataPoint;
    int i, j;

    double sumOut = 0.0;
    
    for (i = 0; i < dataSize; i++)
    {
        dataPoint = dataSet->dataSet[i];
        sumIn = 0.0;
        
        for (j = 0; j < nComp; j++)
        {
            gauss = gmm->comp[j];
            sumIn += (gauss->mix) * gaussProb (gauss, dataPoint);
        }

        sumOut += log (sumIn);
    }

    gmm->like[it] = sumOut;
}











void EStep (GMM* gmm)
{
    int dataSize = gmm->trainData->size;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    double* dataPoint;
    double** dataSet = gmm->trainData->dataSet;
    Gauss* gauss;
    int i, j;

//    #pragma omp parallel for private(i, j, dataPoint, gauss)
    for (i = 0; i < dataSize; i++)
    {
        dataPoint = dataSet[i];
        for (j = 0; j < nComp; j++)
        {
            gauss = comp[j];
            gmm->resp[i*nComp+j] = resp (gmm, gauss, dataPoint);
        }
    }
}








double resp (GMM* gmm, Gauss* gauss, double* dataPoint)
{
    int k;
    Gauss* gaussK;
    double mix = gauss->mix;
    double resp = mix * gaussProb (gauss, dataPoint);
    int nComp = gmm->nComp;
    double sum = 0.0;

    for (k = 0; k < nComp; k++)
    {
        gaussK = gmm->comp[k];
        sum += (gaussK->mix) * gaussProb (gaussK, dataPoint);
    }
    resp /= sum;
    return resp;
}








void MStep (GMM* gmm)
{
    updatePop (gmm);
    updateMix (gmm);
    updateMean (gmm);
    updateCov (gmm);
    
    int i;
    int nComp = gmm->nComp;
    Gauss* gauss;
    int dim = gmm->dim;

    /* Compute and store new covariance determinant, inverse covariance and Gaussian probability coefficient to avoid redundant computation. */
    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];
        gauss->covDet = detGauss (gauss->cov, dim);
        gauss->covInv = inv (gauss->cov, dim, gauss->covDet);
        gauss->probCoef = 1.0 / pow (pow(2*M_PI, dim)*gauss->covDet, 0.5);
    }
}








void updatePop (GMM* gmm)
{
    double* resp = gmm->resp;
    int nComp = gmm->nComp;
    int dataSize = gmm->trainData->size;
    int i, j;
    double pop;
    
    for (j = 0; j < nComp; j++)
    {
        pop = 0.0;
//        #pragma omp parallel for private(i)
        for (i = 0; i < dataSize; i++)
        {
            pop += resp[i*nComp+j];
        }
        gmm->comp[j]->pop = pop;
    }
}









void updateMean (GMM* gmm)
{
    int nComp = gmm->nComp;
    int dataSize = gmm->trainData->size;
    int dim = gmm->dim;
    Gauss* gauss;
    double* mean;
    double* dataPoint;
    double** dataSet = gmm->trainData->dataSet;
    double* resp = gmm->resp;
    Gauss** comp = gmm->comp;
    int i, j;

    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];
        mean = gauss->mean;
        set (mean, 1, dim, 0.0);

//        #pragma omp parallel for private(j, dataPoint)
        for (j = 0; j < dataSize; j++)
        {
            dataPoint = dataSet[j];
            double* prod = multOut (dataPoint, 1, dim, resp[j*nComp+i]);
            addIn (mean, prod, 1, dim);
            free (prod);
        }

        multIn (mean, 1, dim, 1.0/(gauss->pop));
    }
    
}








void updateCov (GMM* gmm)
{
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    int dataSize = gmm->trainData->size;
    int dim = gmm->dim;
    Gauss* gauss;
    double* mean;
    double* resp = gmm->resp;
    double* cov;
    double* dataPoint;
    double** dataSet = gmm->trainData->dataSet;
    int i, j;


    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];

        mean = gauss->mean;
        cov = gauss->cov;
        set (cov, dim, dim, 0.0);

//        #pragma omp parallel for private(j, dataPoint)
        for (j = 0; j < dataSize; j++)
        {
            dataPoint = dataSet[j];
            double* dif = subOut (dataPoint, mean, 1, dim);
            double* prod = dot (dif, dif, dim, 1, 1, dim);
            multIn (prod, dim, dim, resp[j*nComp+i]);
            addIn (cov, prod, dim, dim);
            
            free (dif);
            free (prod);
        }

        multIn (cov, dim, dim, 1.0/(gauss->pop));
    }
}







void updateMix (GMM* gmm)
{
    int dataSize = gmm->trainData->size;
    int nComp = gmm->nComp;
    Gauss* gauss;
    int i;

    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];
        gauss->mix = (double)gauss->pop / (double)dataSize;
    }
}








