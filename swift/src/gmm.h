/* gmm.h */

#ifndef _gmm_h
#define _gmm_h



#include "data.h"
#include "sort.h"

typedef struct
{
    double mix; /* Mixing coefficient. */
    double* mean;   /* Means. */
    double* cov;   /* Covariance matrix. */
    double popSample;   /* Number of sample data points classified to this component. */
    double popTrain;    /* Number of train data points classified to this component. */
    int dim;    /* Dimensions. */
    int fixed;   /* Whether parameters of this gauss should not be updated. */


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
    double* respSample;   /* Responsibilities for each sample data point on each component. */
    double* respTrain;   /* Responsibilities for each train data point on each component. */
    double* likeTrain;    /* Recording history log likelihood of train data set on gmm. */

    int it; /* Number of iterations used to train GMM. */
    double eps; /* Significant digits used to determine convergence. */

    int fixRate;    /* Number of components to fix at a iteration. */
    int nFixed; /* Number of components already been fixed. */

    DataSet* trainData; /* Whole train data set from file. */
    double* dataWeight; /* Weight of each train data point for re-sampling. */
    int sampleSize;  /* Number of data points used for training at each iteration. */
    double** sample;    /* A subset of train data picked for training at each iteration. */
} GMM;








/* Initialize GMM. */
GMM* initGMM (DataSet* trainData, int sampleSize, int nComp, int fixRate, int it, double eps);

/* Initialize mean for each component by randomly select a data point for each component. */
double** randSample (GMM* gmm, int sampleSize);

/* Initialize covariance matrix. 
 * Compute the covariance matrix of sample. */
double* initCov (GMM* gmm);

/* Initialize Gauss. */
Gauss* initGauss (int dim, double* mean, double* cov, double mix);






/* Train GMM using iterative sampling based EM. */
int train (GMM* gmm);




/* Select a weighted randomg sample from train data set. */
void resample (GMM* gmm);

/* Compute weight for each data point in train data set for re-sampling. */
void weightData (GMM* gmm);

/* Fix parameters of most populated components from all unfixed components.
 * Return the number of components actually been fixed at this time. */
int fixComp (GMM* gmm);

/* Adjust mixing coefficients for all unfixed components. */
void adjMix (GMM* gmm);




/* Train GMM using EM algorithm.
 * Return whether converge. */
int EM (GMM* gmm);



/* Return whether given gmm converge based on the records of log likelihood. */
int isConverge (GMM* gmm, int it);


/* Compute probability of given data point on given gauss. */
double gaussProb (Gauss* gauss, double* dataPoint);

/* Compute and update log likelihood of train sample data set. */
void likeTrain (GMM* gmm, int it);




/* Compute responsibilities for each sample data point on each component. */
void EStepSample (GMM* gmm);

/* Compute responsibilities for each train data point on each component. */
void EStepTrain (GMM* gmm);

/* Compute responsiblity for given data point on given gauss. */
double resp (GMM* gmm, Gauss* gauss, double* dataPoint);





/* Compute new mean, covariance and mixing for each component. */
void MStep (GMM* gmm);

/* Update population for each component based on sample responsibilities. */
void updatePopSample (GMM* gmm);

/* Update population for each component based on train data responsibilities. */
void updatePopTrain (GMM* gmm);

/* Update mean for each unfixed component. */
void updateMean (GMM* gmm);

/* Update covariance for each unfixed component. */
void updateCov (GMM* gmm);

/* Update mixing coefficient for each unfixed component. */
void updateMix (GMM* gmm);

/* Update covariance determinant for each unfixed component. */
void updateCovDet (GMM* gmm);

/* Update covariance inverse for each unfixed component. */
void updateCovInv (GMM* gmm);

/* Update probability coefficient for each unfixed component. */
void updateProbCoef (GMM* gmm);


#endif
