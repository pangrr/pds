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
    gmm->resp = (double*) malloc (trainData->size*nComp*sizeof(double));
    gmm->sampleLike = (double*) malloc (it*sizeof(double));
    gmm->prob = (double*) malloc (trainData->size*nComp*sizeof(double));



    /* Initialize components. */
    double* cov = initCov (gmm);
    double mix = 1.0 / nComp;
    int* randMean = randSample (gmm, nComp);

    int i;
    for (i = 0; i < nComp; i++)
    {
        gmm->comp[i] = initGauss (gmm->dim, trainData->dataSet[randMean[i]], cov, mix);
    }
    
    return gmm;
}





int* randSample (GMM* gmm, int sampleSize)
{
    int* randIndex = (int*) malloc (sampleSize*sizeof(int));
    int trainSize = gmm->trainData->size;
    double* order = (double*) malloc (trainSize*sizeof(double));
    int* index = (int*) malloc (trainSize*sizeof(int));
    int i;

    for (i = 0; i < trainSize; i++)
    {
        order[i] = (double)rand() / (double)RAND_MAX;
        index[i] = i;
    }

    quickSort (index, order, 0, trainSize-1);

    for (i = 0; i < sampleSize; i++)
    {
        randIndex[i] = index[i];
    }

    free (order);
    free (index);
    return randIndex;
}




/* Generate a covariance matrix of the whole train data set. */
double* initCov (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    double** trainDataSet = gmm->trainData->dataSet;
    int* sample = gmm->sample;
    int dim = gmm->dim;
    double* cov = (double*) malloc (dim*dim*sizeof(double));
    double* mean = (double*) malloc (dim*sizeof(double));
    int i;


    /* Compute mean of all data points. */
    set (mean, 1, dim, 0.0);
    for (i = 0; i < sampleSize; i++)
    {
        addIn (mean, trainDataSet[sample[i]], 1, dim);
    }
    multIn (mean, 1, dim, 1.0/sampleSize);


    /* Compute covariance matrix of all data points. */
    set (cov, dim, dim, 0.0);
    for (i = 0; i < sampleSize; i++)
    {
        double* sub = subOut (trainDataSet[sample[i]], mean, 1, dim);
        double* mult = dot (sub, sub, dim, 1, 1, dim);
        addIn (cov, mult, dim, dim);
        
        free (sub);
        free (mult);
    }
    multIn (cov, dim, dim, 1.0/sampleSize);

    free (mean);
    return cov;
}




/* Generate a unit covariance matrix. */
//double* initCov (GMM* gmm)
//{
//    int dim = gmm->dim;
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
    gauss->samplePop = 0.0;
    gauss->trainPop = 0.0;
    gauss->dim = dim;
    gauss->fixed = 0;

    gauss->mean = copyOut (mean, 1, dim);
    gauss->cov = copyOut (cov, dim, dim);
    
    gauss->covDet = det (gauss->cov, dim);
    gauss->covInv = inv (gauss->cov, dim);
    gauss->probCoef = pow (pow(2*M_PI, dim)*gauss->covDet, 0.5);
    
    //printf ("%f ", gauss->probCoef);
    //printDoubleArray (gauss->covInv, dim , dim);
    
    return gauss;
}









int train (GMM* gmm)
{
//    int j;
//    for (j = 0; j < gmm->nComp; j++)
//        printDoubleArray (gmm->comp[j]->cov, gmm->dim, gmm->dim);
//    printf ("\n");
//    return 0;

    
    
    EM (gmm);
    return 0;


    updateTrainProb (gmm);
    updateTrainResp (gmm);
    updateTrainPop (gmm);
    
    while (1)
    {
        fixComp (gmm);
    //    printf ("\t%d fixed\n", gmm->nFixed);
        if (gmm->nFixed == gmm->nComp)
        {
            break;
        }
        
        resample (gmm);
        
        //printDoubleArray (gmm->resp, gmm->trainData->size, gmm->nComp);
        //printIntArray (gmm->sample, 1, gmm->sampleSize);
        //for (j = 0; j < gmm->nComp; j++)
        //    printf ("%d ", gmm->comp[j]->fixed);
        
        EM (gmm);
        
//        for (j = 0; j < gmm->nComp; j++)
//            printf ("%f ", gmm->comp[j]->mix);
//        printf ("\n");
        
        adjMix (gmm);
        
//        for (j = 0; j < gmm->nComp; j++)
//            printf ("%f ", gmm->comp[j]->mix);
//        printf ("\n");
//        return 0;

        updateTrainProb (gmm);
        updateTrainResp (gmm);
        updateTrainPop (gmm);
    }
    return 0;
}













void resample (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    int trainSize = gmm->trainData->size;
    int* sample = gmm->sample;
    double* weight = gmm->dataWeight;
    double* order = (double*) malloc (trainSize*sizeof(double));
    int* index = (int*) malloc (trainSize*sizeof(double));
    int i;
    
    weightData (gmm);
    
    for (i = 0; i < trainSize; i++)
    {
        order[i] = pow ((double)rand()/(double)RAND_MAX, 1.0/weight[i]);
        index[i] = i;
    }

    quickSort (index, order, 0, sampleSize-1);

    for (i = 0; i < sampleSize; i++)
    {
        sample[i] = index[i];
    }

    free (order);
    free (index);    
}






void weightData (GMM* gmm)
{
    int dataSize = gmm->trainData->size;
    double* resp = gmm->resp;
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
            order[j] = gauss->trainPop;
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
    updateSampleProb (gmm);
//    int j;
//    for (j = 0; j < gmm->sampleSize; j++)
//    {
//        if (fabs(gmm->prob[gmm->sample[j]]) > 1e-10)
//            printf ("%f ", gmm->prob[gmm->sample[j]]);
//    }
//    return 0;
    return 0;


    for (i = 0; i < it; i++)
    {
        /* E step. */
        updateSampleResp (gmm);
        
        updateSamplePop (gmm);
        
        /* M step. */
        updateMix (gmm);
        updateMean (gmm);
        updateCov (gmm);
        
        updateCovDet (gmm);
        updateCovInv (gmm);
        updateProbCoef (gmm);
        
        updateSampleProb (gmm);

        updateSampleLike (gmm, i);
        //printf ("%d\t%f\n", i, gmm->sampleLike[i]);
        
        if (isConverge(gmm, i) == 1)
        {
            return 1;
        }
    }
    return 0;
    
    
//    //printIntArray (gmm->sample, 1, gmm->sampleSize);
//    int j;
//    for (j = 0; j < gmm->nComp; j++)
//        //printDoubleArray (gmm->comp[j]->mean, 1, gmm->dim);
//        //printDoubleArray (gmm->comp[j]->covInv, gmm->dim, gmm->dim);
//        //printf ("%f ", gmm->comp[j]->probCoef);
//    printDoubleArray (gmm->prob, 1, gmm->trainData->size*gmm->nComp);
//    return 0;
}









int isConverge (GMM* gmm, int it)
{
    if (it > 0)
    {
        double new = gmm->sampleLike[it];
        double old = gmm->sampleLike[it-1];
        if (fabs(new-old) < gmm->eps)
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

    double prob = exp(-(*partA)/2.0) / gauss->probCoef;
    printf ("%.16f ", prob);

    free (sub);
    free (partA);
    return prob;
}





void updateSampleLike (GMM* gmm, int it)
{
    int sampleSize = gmm->sampleSize;
    int* sample = gmm->sample;
    int dataPointIndex;
    double sumIn;
    double* prob = gmm->prob;
    int nComp = gmm->nComp;
    int i, j;
    double sumOut = 0.0;

    for (i = 0; i < sampleSize; i++)
    {
        dataPointIndex = sample[i];
        sumIn = 0.0;
        for (j = 0; j < nComp; j++)
        {
            sumIn += prob[dataPointIndex*nComp+j];
        }
        sumOut += log (sumIn);
    }
    gmm->sampleLike[it] = sumOut;
}




void updateTrainProb (GMM* gmm)
{
    double* prob = gmm->prob;
    int sampleSize = gmm->sampleSize;
    int trainSize = gmm->trainData->size;
    double** trainDataSet = gmm->trainData->dataSet;
    int nComp = gmm->nComp;
    int* sample = gmm->sample;
    double* dataPoint;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    int i, j, k;
    int inSample;

    for (i = 0; i < trainSize; i++)
    {
        inSample = 0;
        for (k = 0; k < sampleSize; k++)
        {
            if (i == sample[k])
            {
                inSample = 1;
                break;
            }
        }

        if (inSample == 0)
        {
            dataPoint = trainDataSet[i];
            for (j = 0; j < nComp; j++)
            {
                gauss = comp[j];
                prob[i*nComp+j] = gauss->mix * gaussProb (gauss, dataPoint);
            }
        }
    }
}










void updateSampleProb (GMM* gmm)
{
    double* prob = gmm->prob;
    int sampleSize = gmm->sampleSize;
    int* sample = gmm->sample;
    double** dataSet = gmm->trainData->dataSet;
    int nComp = gmm->nComp;
    double* dataPoint;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    int i, j, dataPointIndex;

    for (i = 0; i < sampleSize; i++)
    {
        dataPointIndex = sample[i];
        dataPoint = dataSet[dataPointIndex];

        for (j = 0; j < nComp; j++)
        {
            gauss = comp[j];
            prob[dataPointIndex*nComp+j] = gauss->mix * gaussProb (gauss, dataPoint);
        }
    }
}








void updateTrainResp (GMM* gmm)
{
    int dataSize = gmm->trainData->size;
    int nComp = gmm->nComp;
    double* resp = gmm->resp;
    double* prob = gmm->prob;
    int i, j;
    double sumProb;

    
    for (i = 0; i < dataSize; i++)
    {
        sumProb = 0.0;

        for (j = 0; j < nComp; j++)
        {
            sumProb += prob[i*nComp+j];
        }
        
        for (j = 0; j < nComp; j++)
        {
            resp[i*nComp+j] = prob[i*nComp+j] / sumProb;
        }
    }
}






void updateSampleResp (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    int nComp = gmm->nComp;
    int* sample = gmm->sample;
    double* resp = gmm->resp;
    int i, j, dataPointIndex;
    double sumProb;
    double* prob = gmm->prob;

    for (i = 0; i < sampleSize; i++)
    {
        dataPointIndex = sample[i];
        sumProb = 0.0;

        for (j = 0; j < nComp; j++)
        {
            sumProb += prob[dataPointIndex*nComp+j];
        }
        
        for (j = 0; j < nComp; j++)
        {
            resp[dataPointIndex*nComp+j] = prob[dataPointIndex*nComp+j] / sumProb;
        }
    }
}








//double resp (GMM* gmm, Gauss* gauss, double* dataPoint)
//{
//    int k;
//    Gauss* gaussK;
//    double mix = gauss->mix;
//    double resp = mix * gaussProb (gauss, dataPoint);
//    int nComp = gmm->nComp;
//    double sum = 0.0;
//
//    for (k = 0; k < nComp; k++)
//    {
//        gaussK = gmm->comp[k];
//        sum += (gaussK->mix) * gaussProb (gaussK, dataPoint);
//    }
//
//    resp /= sum;
//
//    return resp;
//}













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
            gauss->covDet = det (gauss->cov, dim);
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
            gauss->covInv = inv (gauss->cov, dim);
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
            gauss->probCoef = pow (pow(2*M_PI, dim)*gauss->covDet, 0.5);
        }
    }   
}







void updateSamplePop (GMM* gmm)
{
    double* resp = gmm->resp;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    double pop;
    int sampleSize = gmm->sampleSize;
    int* sample = gmm->sample;
    int i, j;
    
    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];

        if (gauss->fixed == 0)
        {
            pop = 0.0;

            for (j = 0; j < sampleSize; j++)
            {
                pop += resp[sample[j]*nComp+i];
            }

            gauss->samplePop = pop;
        }
    }
}




void updateTrainPop (GMM* gmm)
{
    double* resp = gmm->resp;
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

            gauss->trainPop = pop;
        }
    }
}





void updateMean (GMM* gmm)
{
    int nComp = gmm->nComp;
    int sampleSize = gmm->sampleSize;
    double** dataSet = gmm->trainData->dataSet;
    int dim = gmm->dim;
    int dataPointIndex;
    double* resp = gmm->resp;
    int* sample = gmm->sample;
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
                dataPointIndex = sample[j];
                dataPoint = dataSet[dataPointIndex];
                double* prod = multOut (dataPoint, 1, dim, resp[dataPointIndex*nComp+i]);
                addIn (mean, prod, 1, dim);
                free (prod);
            }

            multIn (mean, 1, dim, 1.0/(gauss->samplePop));
        }
    }
    
}








void updateCov (GMM* gmm)
{
    int nComp = gmm->nComp;
    double** trainDataSet = gmm->trainData->dataSet;
    int sampleSize = gmm->sampleSize;
    int* sample = gmm->sample;
    double* dataPoint;
    int dataPointIndex;
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
                dataPointIndex = sample[j];
                dataPoint = trainDataSet[dataPointIndex];
                double* dif = subOut (dataPoint, gauss->mean, 1, dim);
                double* prod = dot (dif, dif, dim, 1, 1, dim);
                multIn (prod, dim, dim, gmm->resp[dataPointIndex*nComp+i]);
                addIn (cov, prod, dim, dim);
                
                free (dif);
                free (prod);
            }

            multIn (cov, dim, dim, 1.0/(gauss->samplePop));
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
            gauss->mix = (double)gauss->samplePop / (double)sampleSize;
        }
    }
}



