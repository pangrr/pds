/* gmm.c */

#include "gmm.h"
#include "sort.h"
#include "linalg.h"
#include "global.h"
#include "data.h"





GMM* initGMM (DataSet* trainData, int sampleSize, int nComp, int fixRate, int it, double eps)
{
    GMM* gmm = (GMM*) malloc (sizeof(GMM));
    gmm->trainData = trainData;
    gmm->sampleSize = sampleSize;
    gmm->sample = randSample (gmm, sampleSize);
    gmm->dataWeight = (double*) malloc (trainData->size*sizeof(double));

    gmm->comp = (Gauss**) malloc (nComp*sizeof(Gauss*));
    gmm->dim = trainData->dim;
    gmm->nComp = nComp;

    gmm->fixRate = fixRate;
    gmm->nFixed = 0;
    gmm->it = it;
    gmm->eps = eps;
    gmm->respSample = (double*) malloc (sampleSize*nComp*sizeof(double));
    gmm->respTrain = (double*) malloc (trainData->size*nComp*sizeof(double));
    gmm->likeTrain = (double*) malloc (it*sizeof(double));

    /* Initialize components. */
    double* cov = initCov (gmm);
    double mix = 1.0 / nComp;
    double** randMean = randSample (gmm, nComp);

    int i;
    for (i = 0; i < nComp; i++)
    {
        gmm->comp[i] = initGauss (gmm->dim, randMean[i], cov, mix);
    }
    
    return gmm;
}





double** randSample (GMM* gmm, int sampleSize)
{
    double** res = (double**) malloc (sampleSize*sizeof(double));
    double** dataSet = gmm->trainData->dataSet;

    double* order = (double*) malloc (sampleSize*sizeof(double));
    int* index = (int*) malloc (sampleSize*sizeof(double));
    int i;

    for (i = 0; i < sampleSize; i++)
    {
        order[i] = (double)rand() / (double)RAND_MAX;
        index[i] = i;
    }

    quickSort (index, order, 0, sampleSize-1);

    for (i = 0; i < sampleSize; i++)
    {
        res[i] = dataSet[index[i]];
    }

    free (order);
    free (index);

    return res;
}






double* initCov (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    double** sample = gmm->sample;
    int dim = gmm->dim;
    double* cov = (double*) malloc (dim*dim*sizeof(double));
    double* mean = (double*) malloc (dim*sizeof(double));
    int i;


    /* Compute mean of all data points. */
    set (mean, 1, dim, 0.0);
    for (i = 0; i < sampleSize; i++)
    {
        addIn (mean, sample[i], 1, dim);
    }
    multIn (mean, 1, dim, 1.0/sampleSize);


    /* Compute covariance matrix of all data points. */
    set (cov, dim, dim, 0.0);
    for (i = 0; i < sampleSize; i++)
    {
        double* sub = subOut (sample[i], mean, 1, dim);
        double* mult = dot (sub, sub, dim, 1, 1, dim);
        addIn (cov, mult, dim, dim);
        
        free (sub);
        free (mult);
    }
    multIn (cov, dim, dim, 1.0/sampleSize);

    free (mean);
    return cov;
}

//double* initCov (int dim)
//{
//    double* cov = (double*) malloc (dim*dim*sizeof(double));
//    int i, j;
//    for (i = 0; i < dim; i++)
//    {
//        for (j = 0; j < dim; j++)
//        {
//            if (i == j)
//            {
//                cov[i*dim+j] = 1.0;
//            }
//            else
//            {
//                cov[i*dim+j] = 0.0;
//            }
//        }
//    }
//    return cov;
//}







Gauss* initGauss (int dim, double* mean, double* cov, double mix)
{
    Gauss* gauss = (Gauss*) malloc (sizeof(Gauss));

    gauss->mix = mix;
    gauss->popSample = 0.0;
    gauss->popTrain = 0.0;
    gauss->dim = dim;
    gauss->fixed = 0;

    gauss->mean = copyOut (mean, 1, dim);
    gauss->cov = copyOut (cov, dim, dim);
    
    gauss->covDet = detGauss (gauss->cov, dim);
    gauss->covInv = inv (gauss->cov, dim, gauss->covDet);
    gauss->probCoef = 1.0 / pow (pow(2*M_PI, dim)*gauss->covDet, 0.5);
    
    return gauss;
}









int train (GMM* gmm)
{
    EM (gmm);
    EStepTrain (gmm);
    updatePopTrain (gmm);
    
    while (1)
    {
        fixComp (gmm);
        printf ("\t%d fixed\n", gmm->nFixed);
        if (gmm->nFixed == gmm->nComp)
        {
            break;
        }
        resample (gmm);
        EM (gmm);
        adjMix (gmm);
        EStepTrain (gmm);
        updatePopTrain (gmm);
    }
    return 0;
}













void resample (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    double** sample = gmm->sample;
    double** dataSet = gmm->trainData->dataSet;
    double* weight = gmm->dataWeight;
    double* order = (double*) malloc (sampleSize*sizeof(double));
    int* index = (int*) malloc (sampleSize*sizeof(double));
    int i;
    
    weightData (gmm);
    
    for (i = 0; i < sampleSize; i++)
    {
        order[i] = pow ((double)rand()/(double)RAND_MAX, 1.0/weight[i]);
        index[i] = i;
    }

    quickSort (index, order, 0, sampleSize-1);

    for (i = 0; i < sampleSize; i++)
    {
        sample[i] = dataSet[index[i]];
    }

    free (order);
    free (index);    
}






void weightData (GMM* gmm)
{
    int dataSize = gmm->trainData->size;
    double* resp = gmm->respTrain;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    double* weight = gmm->dataWeight;
    double w;
    int i, j;

    for (i = 0; i < dataSize; i++)
    {
        w = 0.0;
        for (j = 0; j < nComp; j++)
        {
            gauss = comp[j];
            if (gauss->fixed == 0)
            {
                w += resp[i*nComp+j];
            }
        }
        weight[i] = w;
    }
}







int fixComp (GMM* gmm)
{
    int nComp = gmm->nComp;
    int nUnfixed = nComp - gmm->nFixed;
    int fixRate = gmm->fixRate;
    int i, j;
    Gauss** comp = gmm->comp;
    Gauss* gauss;

    if (nUnfixed == 0)
    {
        return 0;
    }
    

    double* order = (double*) malloc (nUnfixed*sizeof(double));
    int* index = (int*) malloc (nUnfixed*sizeof(double));

    /* Prepare for sort by building order for each unfixed component. */
    j = 0;
    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];
        if (gauss->fixed == 0)
        {
            order[j] = gauss->popTrain;
            index[j] = i;
            j++;
        }
    }
    /* Sort unfixed components according to their population. */
    quickSort (index, order, 0, nUnfixed-1);

    /* Try to fix some components with higher order. */
    i = 0;
    while (fixRate > 0 && nUnfixed > 0)
    {
        gauss = comp[index[i]];
        gauss->fixed = 1;
        
        fixRate--;
        nUnfixed--;
        i++;
    }
    gmm->nFixed = nComp - nUnfixed;
    free (order);
    free (index);
    return i;
}








void adjMix (GMM* gmm)
{
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    int nComp = gmm->nComp;
    int i;
    
    double targetSum = 1.0;
    double oldSum = 0.0;
    double adj;

    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];
        if (gauss->fixed == 0)
        {
            oldSum += gauss->mix;
        }
        else
        {
            targetSum -= gauss->mix;
        }
    }

    adj = targetSum / oldSum;

    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];
        if (gauss->fixed == 0)
        {
            gauss->mix *= adj;
        }
    }
}








int EM (GMM* gmm)
{
    int i;
    int it = gmm->it;

    for (i = 0; i < it; i++)
    {
        EStepSample (gmm);
        MStep (gmm);
        likeTrain (gmm, i);
        printf ("%d\t%f\n", i, gmm->likeTrain[i]);
        if (isConverge(gmm, i) == 1)
        {
            return 1;
        }
    }
    return 0;
}









int isConverge (GMM* gmm, int it)
{
    if (it > 0)
    {
        double new = gmm->likeTrain[it];
        double old = gmm->likeTrain[it-1];
        if (fabs((new-old)/old) < gmm->eps)
        {
            return 1;
        }
    }
    return 0;
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





void likeTrain (GMM* gmm, int it)
{
    int sampleSize = gmm->sampleSize;
    double** sample = gmm->sample;
    double* dataPoint;
    double sumIn;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    int i, j;
    double sumOut = 0.0;
    
    for (i = 0; i < sampleSize; i++)
    {
        dataPoint = sample[i];
        sumIn = 0.0;
        
        for (j = 0; j < nComp; j++)
        {
            gauss = comp[j];
            sumIn += gauss->mix * gaussProb (gauss, dataPoint);
        }

        sumOut += log (sumIn);
    }
    gmm->likeTrain[it] = sumOut;
}





void EStepTrain (GMM* gmm)
{
    int dataSize = gmm->trainData->size;
    int nComp = gmm->nComp;
    double** dataSet = gmm->trainData->dataSet;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    double* respTrain = gmm->respTrain;
    double* dataPoint;
    int i, j;

    for (i = 0; i < dataSize; i++)
    {
        dataPoint = dataSet[i];

        for (j = 0; j < nComp; j++)
        {
            gauss = comp[j];
            respTrain[i*nComp+j] = resp (gmm, gauss, dataPoint);
        }
    }
}






void EStepSample (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    int nComp = gmm->nComp;
    double** sample = gmm->sample;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    double* respSample = gmm->respSample;
    double* dataPoint;
    int i, j;

    for (i = 0; i < sampleSize; i++)
    {
        dataPoint = sample[i];

        for (j = 0; j < nComp; j++)
        {
            gauss = comp[j];
            respSample[i*nComp+j] = resp (gmm, gauss, dataPoint);
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
    updatePopSample (gmm);
    updateMix (gmm);
    updateMean (gmm);
    updateCov (gmm);

    updateCovDet (gmm);
    updateCovInv (gmm);
    updateProbCoef (gmm);
}






void updateCovDet (GMM* gmm)
{
    Gauss* gauss;
    int dim = gmm->dim;
    int i;

    for (i = 0; i < gmm->nComp; i++)
    {
        gauss = gmm->comp[i];
        if (gauss->fixed == 0)
        {
            gauss->covDet = detGauss (gauss->cov, dim);
        }
    }
}







void updateCovInv (GMM* gmm)
{
    Gauss* gauss;
    int dim = gmm->dim;
    int i;

    for (i = 0; i < gmm->nComp; i++)
    {
        gauss = gmm->comp[i];
        if (gauss->fixed == 0)
        {
            gauss->covInv = inv (gauss->cov, dim, gauss->covDet);
        }
    }

}







void updateProbCoef (GMM* gmm)
{
    Gauss* gauss;
    int dim = gmm->dim;
    int i;
    
    for (i = 0; i < gmm->nComp; i++)
    {
        gauss = gmm->comp[i];
        if (gauss->fixed == 0)
        {
            gauss->probCoef = 1.0 / pow (pow(2*M_PI, dim)*gauss->covDet, 0.5);
        }
    }   
}







void updatePopSample (GMM* gmm)
{
    double* resp = gmm->respSample;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    double pop;
    int sampleSize = gmm->sampleSize;

    int i, j;
    
    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];

        if (gauss->fixed == 0)
        {
            pop = 0.0;

            for (j = 0; j < sampleSize; j++)
            {
                pop += resp[j*nComp+i];
            }


            gauss->popSample = pop;
        }
    }
}




void updatePopTrain (GMM* gmm)
{
    double* resp = gmm->respTrain;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    double pop;
    int dataSize = gmm->trainData->size;

    int i, j;
    
    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];

        if (gauss->fixed == 0)
        {
            pop = 0.0;

            for (j = 0; j < dataSize; j++)
            {
                pop += resp[j*nComp+i];
            }

            gauss->popTrain = pop;
        }
    }
}





void updateMean (GMM* gmm)
{
    int nComp = gmm->nComp;
    int sampleSize = gmm->sampleSize;
    int dim = gmm->dim;
    double* resp = gmm->respSample;
    double** sample = gmm->sample;
    double* mean;
    Gauss* gauss;
    double* dataPoint;
    int i, j;

    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];

        if (gauss->fixed == 0)
        {
            mean = gauss->mean;
            set (mean, 1, dim, 0.0);

            for (j = 0; j < sampleSize; j++)
            {
                dataPoint = sample[j];
                double* prod = multOut (dataPoint, 1, dim, resp[j*nComp+i]);
                addIn (mean, prod, 1, dim);
                free (prod);
            }

            multIn (mean, 1, dim, 1.0/(gauss->popSample));
        }
    }
    
}








void updateCov (GMM* gmm)
{
    int nComp = gmm->nComp;
    int sampleSize = gmm->sampleSize;
    double** sample = gmm->sample;
    double* dataPoint;
    int dim = gmm->dim;
    double* cov;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    int i, j;


    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];

        if (gauss->fixed == 0)
        {
            cov = gauss->cov;
            set (cov, dim, dim, 0.0);

            for (j = 0; j < sampleSize; j++)
            {
                dataPoint = sample[j];
                double* dif = subOut (dataPoint, gauss->mean, 1, dim);
                double* prod = dot (dif, dif, dim, 1, 1, dim);
                multIn (prod, dim, dim, gmm->respSample[j*nComp+i]);
                addIn (cov, prod, dim, dim);
                
                free (dif);
                free (prod);
            }

            multIn (cov, dim, dim, 1.0/(gauss->popSample));
        }
        
    }
}







void updateMix (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    int i;

    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];
        if (gauss->fixed == 0)
        {
            gauss->mix = (double)gauss->popSample / (double)sampleSize;
        }
    }
}



