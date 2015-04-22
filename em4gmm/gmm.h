/* gmm.h */

#ifndef _gmm_h
#define _gmm_h



#include "data.h"


typedef struct
{
    double mix; /* Mixing coefficient. */
    double* mean;   /* Means. */
    double* cov;   /* Covariance matrix. */
    double pop; /* Number of data points clessified to this component. */
    int dim;    /* Dimensions. */


    /* To avoid repeatative computation. */
    double covDet;
    double* covInv;
    double probCoef;
    
} Gauss;






typedef struct
{
    Gauss** comp;    /* Components. */
    int dim;    /* Dimensions. */
    int nComp;  /* Number of components. */
    double* resp;   /* Responsibilities for each data point on each component. */
    double* like;    /* Recording history log likelihood of training data set on gmm. */

    int it; /* Number of iterations used to train GMM. */
    double eps; /* Threshold used to determine convergence. */

    DataSet* trainData; /* Whole data set from file. */
    int dataSize;  /* Number of data points used for training at each iteration. */
} GMM;








/* Initialize GMM. */
GMM* initGMM (DataSet* dataSet, int nComp, int it, double esp);

/* Return a poniter to a random data point. */
double* randData (DataSet* dataSet);

/* Initialize covariance matrix. 
 * Compute the covariance matrix of whole data set. */
double* initCov (DataSet* dataSet);

/* Initialize Gauss. */
Gauss* initGauss (int dim, double* mean, double* cov, double mix);






/* Train given gmm on given data set using EM algorithm.
 * Return whether converge. */
int trainEM (GMM* gmm);


/* Return whether given gmm converge based on the records of log likelihood. */
int isConverge (GMM* gmm, int it);


/* Compute probability of given data point on given gauss. */
double gaussProb (Gauss* gauss, double* dataPoint);





/* Compute and update log likelihood of data set on gmm. */
void like (GMM* gmm, DataSet* dataSet, int it);



/* Compute responsibilities for each data point on each component. */
void EStep (GMM* gmm);

/* Compute responsiblity for given data point on given gauss. */
double resp (GMM* gmm, Gauss* gauss, double* dataPoint);






/* Compute new mean, covariance and mixing for each component. */
void MStep (GMM* gmm);

/* Compute new frequency for each component. */
void updatePop (GMM* gmm);

/* Compute new mean for each component. */
void updateMean (GMM* gmm);

/* Compute new covariance for each component. */
void updateCov (GMM* gmm);

/* Compute new mixing coefficient for each component. */
void updateMix (GMM* gmm);



#endif
