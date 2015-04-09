/* gmm.h */

#ifndef _gmm_h
#define _gmm_h



#include "data.h"


typedef struct
{
    double mix; /* Mixing coefficient. */
    double* mean;   /* Means. */
    double* cov;   /* Covariance matrix. */
    double _mix;    /* New mixing corfficient. */
    double* _mean;  /* New means. */
    double* _cov;  /* New covariance matrix. */
    int freq; /* Number of data points clessified to this component. */
    int dim;    /* Dimensions. */
    int fix;   /* Whether parameters of this gauss should not be updated. */
} Gauss;






typedef struct
{
    Gauss** comp;    /* Components. */
    int dim;    /* Dimensions. */
    int nComp;  /* Number of components. */
    double* resp;   /* Responsibilities for each data point on each gauss. */
    double* like;    /* Recording history log likelihood of training data set on gmm. */

    int it; /* Number of iterations used to train GMM. */
    double convThre; /* Threshold used to determine convergence. */
    int traceBack; /* Number of likelihoods used to determine convergence. */
} GMM;








/* Initialize GMM. */
GMM* initGMM (DataSet* dataSet, int nComp, int it, double convThre, int traceBack);

/* Return a poniter to a random data point. */
double* randData (DataSet* dataSet);

/* Initialize covariance matrix. 
 * Compute the covariance matrix of whole data set. */
double* initCov (DataSet* dataSet);

/* Initialize Gauss. */
Gauss* initGauss (int dim, double* mean, double* cov, double mix);








/* Train given gmm on given data set using EM algorithm.
 * Return whether converge. */
int trainEM (GMM* gmm, DataSet* dataSet);


/* Return whether given gmm converge based on the records of log likelihood. */
int isConverge (GMM* gmm);


/* Compute probability of given data point on given gauss. */
double gaussProb (Gauss* gauss, DataPoint* dataPoint);









/* Compute and update log likelihood of data set on gmm. */
void like (GMM* gmm, DataSet* dataSet, int it);


/* Update parameters of each gauss in GMM. */
void updateGMM (GMM* gmm);


/* Compute responsibilities for each data point on each gauss. */
void EStep (GMM* gmm, DataSet* dataSet);

/* Compute responsiblity for given data point on given gauss. */
double resp (GMM* gmm, Gauss* gauss, DataPoint* dataPoint);









/* Compute new mean, covariance and mixing for each gauss. */
void MStep (GMM* gmm, DataSet* dataSet);

/* Compute new frequency for each gauss. */
void freq (GMM* gmm, DataSet* dataSet);

/* Compute new mean for each gauss. */
void newMean (GMM* gmm, DataSet* dataSet);

/* Compute new covariance for each gauss. */
void newCov (GMM* gmm, DataSet* dataSet);

/* Compute new mixing coefficient for each gauss. */
void newMix (GMM* gmm, DataSet* dataSet);


/* Print all likelihood records. */
void printLike (GMM* gmm);



#endif
