/* gmm.c */

#include "gmm.h"
#include "sort.h"
#include "global.h"
#include "data.h"
#include "search.h"




GMM* initGMM (DataSet* trainData, int* sample, int sampleSize, double** means, double** covs, int nComp, int fixRate, int it, double eps)
{
    GMM* gmm = (GMM*) malloc (sizeof(GMM));
    gmm->trainData = trainData;
    gmm->sampleSize = sampleSize;
    gmm->sample = sample;
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
    double mix = 1.0 / nComp;

    int i;
    for (i = 0; i < nComp; i++)
    {
        gmm->comp[i] = initGauss (gmm->dim, means[i], covs[i], mix);
    }
    
    return gmm;
}










Gauss* initGauss (int dim, double* mean, double* cov, double mix)
{
    Gauss* gauss = (Gauss*) malloc (sizeof(Gauss));
    gauss->mix = mix;
    gauss->samplePop = 0.0;
    gauss->trainPop = 0.0;
    gauss->dim = dim;
    gauss->fixed = 0;
    gauss->mean = mean;
    gauss->cov = cov;
    gauss->covDet = 1.0;
    int i;
    for (i = 0; i < dim; i++)
    {
        gauss->covDet *= cov[i];
    }
    gauss->probCoef = pow(2*M_PI, dim*0.5) * pow(gauss->covDet, 0.5);
    return gauss;
}











void printGMM (GMM* gmm)
{
    printMix (gmm);
    printMean (gmm);
    printCov (gmm);
}






void printMean (GMM* gmm)
{
    int i, j;
    int dim = gmm->dim;
    int nComp = gmm->nComp;
    double* mean;
    
    printf ("mean:\n");
    for (i = 0; i < nComp; i++)
    {
        mean = gmm->comp[i]->mean;
        for (j = 0; j < dim; j++)
        {
            printf ("%f ", mean[j]);
        }
        printf ("\n");
    }
}


void printCov (GMM* gmm)
{
    int i, j;
    int dim = gmm->dim;
    int nComp = gmm->nComp;
    double* cov;
    
    printf ("cov:\n");
    for (i = 0; i < nComp; i++)
    {
        cov = gmm->comp[i]->cov;
        for (j = 0; j < dim; j++)
        {
            printf ("%f ", cov[j]);
        }
        printf ("\n");
    }
}


void printMix (GMM* gmm)
{
    int i;
    int nComp = gmm->nComp;
    Gauss** comp = gmm->comp;
    Gauss* gauss;
    
    printf ("mix:\n");
    for (i = 0; i < nComp; i++)
    {
        gauss = comp[i];
        printf ("%f ", gauss->mix);
    }
    printf ("\n");
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
    
//    #pragma omp parallel for private(i)
    for (i = 0; i < trainSize; i++)
    {
        order[i] = pow ((double)rand()/(double)RAND_MAX, 1.0/weight[i]);
        index[i] = i;
    }

    quickSortDouble (index, order, 0, trainSize-1);

    for (i = 0; i < sampleSize; i++)
    {
        sample[i] = index[i];
    }

    free (order);

    quickSortInt (index, sample, 0, sampleSize-1);
    free(index);
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

//    #pragma omp parallel for private(i, w, j, gauss)
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
    quickSortDouble (index, order, 0, nUnfixed-1);

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



int trainGMM (GMM* gmm)
{
    EM (gmm);

    updateTrainProb (gmm);
    updateTrainResp (gmm);
    updateTrainPop (gmm);
    
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

        updateTrainProb (gmm);
        updateTrainResp (gmm);
        updateTrainPop (gmm);
    }

    printMix (gmm);
    printCov (gmm);
    printMean (gmm);
    return 0;
}










int EM (GMM* gmm)
{
    int i;
    int it = gmm->it;
    int isConv = 0;

    updateSampleProb (gmm);
    
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
        updateProbCoef (gmm);

        updateSampleProb (gmm);
        
        updateSampleLike (gmm, i);
        printf ("%d\t%f\n", i, gmm->sampleLike[i]);
        
        if (isConverge(gmm, i) == 1)
        {
            isConv = 1;
            break;
        }
    }
//    printMix (gmm);
//    printCov (gmm);
//    printMean (gmm);
    return isConv;
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
    int i;
    double* mean = gauss->mean;
    double* cov = gauss->cov;
    double sum = 0.0;
    double sub;

    for (i = 0; i < dim; i++)
    {
        sub = dataPoint[i] - mean[i];
        sum += sub*sub / cov[i];
    }
    
    double prob = exp(-0.5*sum) / gauss->probCoef;
    
    if (prob < 1e-36)
    {
        prob = 1e-36;   /* Lower bound of prob. */
    }
    return prob;
}





void updateSampleLike (GMM* gmm, int it)
{
    int sampleSize = gmm->sampleSize;
    int* sample = gmm->sample;
    double sumIn;
    double* prob = gmm->prob;
    int nComp = gmm->nComp;
    int i, j;
    double sumOut = 0.0;

    for (i = 0; i < sampleSize; i++)
    {
        sumIn = 0.0;
        for (j = 0; j < nComp; j++)
        {
            sumIn += prob[sample[i]*nComp+j];
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
    int i, j;
    
//    #pragma omp parallel for private(i, dataPoint, j, gauss)
    for (i = 0; i < trainSize; i++)
    { 
        if (biSearch (i, sample, 0, sampleSize-1) == -1)
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
    Gauss* gauss;
    int i, j, dataPointIndex;
    
//    #pragma omp parallel for private(i, dataPointIndex, dataPoint, j, gauss)
    for (i = 0; i < sampleSize; i++)
    {
        dataPointIndex = sample[i];
        dataPoint = dataSet[dataPointIndex];

        for (j = 0; j < nComp; j++)
        {
            gauss = gmm->comp[j];
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

//    #pragma omp parallel for private(i, sumProb, j)
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

//    #pragma omp parallel for private(i, dataPointIndex, sumProb, j)
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
    int i, j;

//    #pragma omp parallel for private(i, gauss, j)
    for (i = 0; i < gmm->nComp; i++)
    {
        gauss = gmm->comp[i];
        if (gauss->fixed == 0)
        {
            gauss->covDet = 1.0;
            
            for (j = 0; j < dim; j++)
            {
                gauss->covDet *= gauss->cov[j];
            }
        }
    }
}




//void updateCovDet (GMM* gmm)
//{
//    Gauss* gauss;
//    int dim = gmm->dim;
//    int i;
//
//    for (i = 0; i < gmm->nComp; i++)
//    {
//        gauss = gmm->comp[i];
//        if (gauss->fixed == 0)
//        {
//            gauss->covDet = det (gauss->cov, dim);
//        }
//    }
//}







//void updateCovInv (GMM* gmm)
//{
//    Gauss* gauss;
//    double* temp;
//    int dim = gmm->dim;
//    int i;
//
//    for (i = 0; i < gmm->nComp; i++)
//    {
//        gauss = gmm->comp[i];
//        if (gauss->fixed == 0)
//        {
//            temp = gauss->covInv;
//            gauss->covInv = inv (gauss->cov, dim);
//            free (temp);
//        }
//    }
//
//}







void updateProbCoef (GMM* gmm)
{
    Gauss* gauss;
    int dim = gmm->dim;
    int i;
    
    for (i = 0; i < gmm->nComp; i++)
    {
        gauss = gmm->comp[i];
        
        //printf ("%f ", gauss->probCoef);
        
        if (gauss->fixed == 0)
        {
            gauss->probCoef = pow(2*M_PI, dim*0.5) * pow(gauss->covDet, 0.5);
        }
    }
    //printf ("\n");
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
    
//    #pragma omp parallel for private(i, gauss, pop, j)
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
    
//    #pragma omp parallel for private(i, gauss, pop, j)
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
    double* mean;
    Gauss* gauss;
    double* dataPoint;
    double resp;
    int* sample = gmm->sample;
    int i, j, k;

//    #pragma omp parallel for private(i, gauss, mean, j, dataPoint, resp, k)
    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];

        if (gauss->fixed == 0)
        {
            mean = gauss->mean;
            resetArray (mean, dim);

            for (j = 0; j < sampleSize; j++)
            {
                dataPoint = dataSet[sample[j]];
                resp = gmm->resp[sample[j]*nComp+i];
                
                for (k = 0; k < dim; k++)
                {
                    mean[k] += resp * dataPoint[k];
                }
            }

            for (k = 0; k < dim; k++)
            {
                mean[k] /= gauss->samplePop;
            }
        }
    } 
}



void resetArray (double* A, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        A[i] = 0.0;
    }
}





void updateCov (GMM* gmm)
{
    int nComp = gmm->nComp;
    double** trainDataSet = gmm->trainData->dataSet;
    int sampleSize = gmm->sampleSize;
    int* sample = gmm->sample;
    double* dataPoint;
    int dim = gmm->dim;
    double* cov;
    Gauss* gauss;
    int i, j, k;
    double sub, resp;
    double* mean;
    
//    #pragma omp parallel for private(i, gauss, mean, cov, j, dataPoint, resp, k, sub)
    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];

        if (gauss->fixed == 0)
        {
            mean = gauss->mean;
            cov = gauss->cov;
            resetArray (cov, dim);

            for (j = 0; j < sampleSize; j++)
            {
                dataPoint = trainDataSet[sample[j]];
                resp = gmm->resp[sample[j]*nComp+i];
                
                for (k = 0; k < dim; k++)
                {
                    sub = dataPoint[k] - mean[k];
                    cov[k] += resp * sub * sub;
                }
            }

            for (k = 0; k < dim; k++)
            {
                cov[k] /= gauss->samplePop;
            }
        }
    }
}







void updateMix (GMM* gmm)
{
    int sampleSize = gmm->sampleSize;
    int nComp = gmm->nComp;
    Gauss* gauss;
    int i;

    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];
        if (gauss->fixed == 0)
        {
            gauss->mix = (double)gauss->samplePop / (double)sampleSize;
        }
    }
}



