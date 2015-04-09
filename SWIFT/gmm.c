/* gmm.c */

#include "gmm.h"
#include "linalg.h"
#include "global.h"
#include "data.h"





GMM* initGMM (DataSet* dataSet, int nComp, int it, double convThre, int traceBack)
{
    int i;
    int dim = dataSet->dim;
    int nData = dataSet->nData;
    double* cov = initCov (dataSet);
    double* mean;
    double mix = 1.0 / nComp;

    GMM* gmm = (GMM*) malloc (sizeof(GMM));
    gmm->comp = (Gauss**) malloc (nComp*sizeof(Gauss*));
    gmm->dim = dim;
    gmm->nComp = nComp;
    gmm->it = it;
    gmm->convThre = convThre;
    gmm->traceBack = traceBack;
    gmm->resp = (double*) malloc (nData*nComp*sizeof(double));
    gmm->like = (double*) malloc (it*sizeof(double));

    /* Initialize components. */
    for (i = 0; i < nComp; i++)
    {
        mean = randData (dataSet);
        gmm->comp[i] = initGauss (dim, mean, cov, mix);
    }
    
    return gmm;
}







double* randData (DataSet* dataSet)
{
    int nData = dataSet->nData;
    int r = rand () % nData;
    return dataSet->data[r]->data;
}





double* initCov (DataSet* dataSet)
{
    int nData = dataSet->nData;
    int dim = dataSet->dim;
    double* cov = (double*) malloc (dim*dim*sizeof(double));
    double* mean = (double*) malloc (dim*sizeof(double));
    int i;


    /* Compute mean of all data points. */
    set (mean, 1, dim, 0.0);
    for (i = 0; i < nData; i++)
    {
        addIn (mean, dataSet->data[i]->data, 1, dim);
    }
    multIn (mean, 1, dim, 1.0/nData);


    /* Compute covariance matrix of all data points. */
    set (cov, dim, dim, 0.0);
    for (i = 0; i < nData; i++)
    {
        double* sub = subOut (dataSet->data[i]->data, mean, 1, dim);
        double* mult = dot (sub, sub, dim, 1, 1, dim);
        addIn (cov, mult, dim, dim);
        
        free (sub);
        free (mult);
    }
    multIn (cov, dim, dim, 1.0/nData);

    free (mean);
    return cov;
}






Gauss* initGauss (int dim, double* mean, double* cov, double mix)
{
    Gauss* gauss = (Gauss*) malloc (sizeof(Gauss));

    gauss->mix = mix;
    gauss->_mix = mix;
    gauss->freq = 0;
    gauss->dim = dim;
    gauss->fix = 0;

    gauss->mean = copyOut (mean, 1, dim);
    gauss->cov = copyOut (cov, dim, dim);
    gauss->_mean = (double*) malloc (dim*sizeof(double));
    gauss->_cov = (double*) malloc (dim*dim*sizeof(double));
    
    return gauss;
}





int trainEM (GMM* gmm, DataSet* dataSet)
{
    int i;
    int it = gmm->it;

    for (i = 0; i < it; i++)
    {
        //printf ("Training %d/%d...\n", i+1, it);
    
        //printf ("E step...\n");
        EStep (gmm, dataSet);

        //printf ("M step...\n");
        MStep (gmm, dataSet);

        //printf ("Updating parameters...\n");
        updateGMM (gmm);

        //printf ("Computing likelihood...\n");
        like (gmm, dataSet, i);

        //printf ("Log likelihood = %f\n", gmm->like[i]);

        if (isConverge(gmm))
            return 1;
    }

    return 0;
}





int isConverge (GMM* gmm)
{
    return 0;
}





double gaussProb (Gauss* gauss, DataPoint* dataPoint)
{
    int dim = gauss->dim;
    double* cov = gauss->cov;
    double* mean = gauss->mean;
    double* data = dataPoint->data;

    /* Temporary memory to be freed. */
    double* sub = subOut (data, mean, 1, dim);
    double* invCov = inv (cov, dim);
    double* partA = dot (dot(sub, invCov, 1, dim, dim, dim), sub, 1, dim, dim, 1);

    //printArray (cov, dim, dim);
    //printf ("det=%.15f\n", det(cov, dim));
    double partB = pow (pow(2*M_PI, dim)*det(cov, dim), 0.5);

    //printf ("partA=%.15f\n", *partA);
    //printf ("partB=%.15f\n", partB);
    
    double prob = exp(-(*partA)/2.0) / partB;

    free (sub);
    free (invCov);
    free (partA);
    
    //printf ("prob = %f\n", prob);
    return prob;
}





void like (GMM* gmm, DataSet* dataSet, int it)
{
    int nData = dataSet->nData;
    int nComp = gmm->nComp;

    double sumIn;
    Gauss* gauss;
    DataPoint* dataPoint;
    int i, j;

    double sumOut = 0.0;
    
    for (i = 0; i < nData; i++)
    {
        dataPoint = dataSet->data[i];
        sumIn = 0.0;
        
        for (j = 0; j < nComp; j++)
        {
            gauss = gmm->comp[j];
            //printf ("%f %f\n", gauss->mix, gaussProb(gauss, dataPoint));
            sumIn += (gauss->mix) * gaussProb (gauss, dataPoint);
        }

        //printf ("%f\n", sumIn);
        sumOut += log (sumIn);
    }

    gmm->like[it] = sumOut;
    printf ("%f\n", sumOut);
}









void updateGMM (GMM* gmm)
{
    int dim = gmm->dim;
    int nComp = gmm->nComp;
    Gauss* gauss;

    int i;
    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];
        copyIn (gauss->mean, gauss->_mean, 1, dim);
        copyIn (gauss->cov, gauss->_cov, dim, dim);
        gauss->mix = gauss->_mix;
    }
}








void EStep (GMM* gmm, DataSet* dataSet)
{
    int nData = dataSet->nData;
    int nComp = gmm->nComp;
    Gauss* gauss;
    DataPoint* dataPoint;
    int i, j;

    for (i = 0; i < nData; i++)
    {
        dataPoint = dataSet->data[i];

        for (j = 0; j < nComp; j++)
        {
            gauss = gmm->comp[j];
            gmm->resp[i*nComp+j] = resp (gmm, gauss, dataPoint);
        }
    }
}








double resp (GMM* gmm, Gauss* gauss, DataPoint* dataPoint)
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








void MStep (GMM* gmm, DataSet* dataSet)
{
    freq (gmm, dataSet);
    newMix (gmm, dataSet);
    newMean (gmm, dataSet);
    newCov (gmm, dataSet);
}








void freq (GMM* gmm, DataSet* dataSet)
{
    double* resp = gmm->resp;
    int nComp = gmm->nComp;
    int nData = dataSet->nData;
    double freq;
    int i, j;

    for (j = 0; j < nComp; j++)
    {
        freq = 0.0;

        for (i = 0; i < nData; i++)
        {
            freq += resp[i*nComp+j];
        }
        gmm->comp[j]->freq = freq;
    }
}









void newMean (GMM* gmm, DataSet* dataSet)
{
    int nComp = gmm->nComp;
    int nData = dataSet->nData;
    int dim = dataSet->dim;
    double* mean;
    Gauss* gauss;
    int i, j;


    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];
        mean = gauss->_mean;
        set (mean, 1, dim, 0.0);

        for (j = 0; j < nData; j++)
        {
            
            double* prod = multOut (dataSet->data[j]->data, 1, dim, gmm->resp[j*nComp+i]);
            addIn (mean, prod, 1, dim);
            free (prod);
        }

        multIn (mean, 1, dim, 1.0/(gauss->freq));
    }
    
}








void newCov (GMM* gmm, DataSet* dataSet)
{
    int nComp = gmm->nComp;
    int nData = dataSet->nData;
    int dim = dataSet->dim;
    Gauss* gauss;
    double* cov;
    int i, j;


    for (i = 0; i < nComp; i++)
    {
        gauss = gmm->comp[i];
        
        cov = gauss->_cov;
        set (cov, dim, dim, 0.0);

        for (j = 0; j < nData; j++)
        {
            double* dif = subOut (dataSet->data[j]->data, gauss->_mean, 1, dim);
            double* prod = dot (dif, dif, dim, 1, 1, dim);
            multIn (prod, dim, dim, gmm->resp[j*nComp+i]);
            addIn (cov, prod, dim, dim);
            
            free (dif);
            free (prod);
        }

        multIn (cov, dim, dim, 1.0/(gauss->freq));
        //printArray (cov, dim, dim);
    }
}







void newMix (GMM* gmm, DataSet* dataSet)
{
    int nData = dataSet->nData;
    int nComp = gmm->nComp;
    int i;

    for (i = 0; i < nComp; i++)
    {
        Gauss* gauss = gmm->comp[i];
        gauss->_mix = (double)gauss->freq / (double)nData;
        //printf ("nData=%d freq=%d _mix=%f\n", nData,  gauss->freq, gauss->_mix);
    }
}








void printLike (GMM* gmm)
{
    int i;
    for (i = 0; i < gmm->it; i++)
    {
        printf ("%f ", gmm->like[i]);
    }
    printf ("\n");
}
