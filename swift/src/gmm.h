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
    double samplePop;   /* Number of sample data points classified to this component. */
    double trainPop;    /* Number of train data points classified to this component. */
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
    double* resp;   /* Responsibilities for each train data point on each component. */
    double* sampleLike;    /* Recording history log likelihood of train data set on gmm. */

    int it; /* Number of iterations used to train GMM. */
    double eps; /* Significant digits used to determine convergence. */

    int fixRate;    /* Number of components to fix at a iteration. */
    int nFixed; /* Number of components already been fixed. */

    DataSet* trainData; /* Whole train data set from file. */
    double* dataWeight; /* Weight of each train data point for re-sampling. */
    /* Number of data points used for training at each iteration. */
    int sampleSize;
    /* A list of index of sample data point in train data set selected for training at each iteration. */
    int* sample;

    /* mix_k * gaussProb_nk for each data point and component. */
    double* prob;   
} GMM;








/* Initialize GMM. */
GMM* initGMM (DataSet* trainData, int* sample, int sampleSize, double** means, double** covs, int nComp, int fixRate, int it, double eps);


/* Initialize Gauss. */
Gauss* initGauss (int dim, double* mean, double* cov, double mix);



void resetArray (double* A, int n);


/* Train GMM using iterative sampling based EM. */
int trainGMM (GMM* gmm);

/* Print parameters of GMM. */
void printGMM (GMM* gmm);
void printMean (GMM* gmm);
void printCov (GMM* gmm);
void printMix (GMM* gmm);

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
void updateSampleLike (GMM* gmm, int it);




/* Compute responsibilities for each sample data point on each component. */
void updateSampleResp (GMM* gmm);

/* Compute responsibilities for each train data point on each component. */
void updateTrainResp (GMM* gmm);


/* Compute and update point probability for each sample data point in each component as well as the sum over all component for each data point. */
void updateSampleProb (GMM* gmm);

/* Compute and update point probability for each data point not in sample in each component as well as the sum over all component for each data point. */
void updateTrainProb (GMM* gmm);


/* Update population for each component based on sample responsibilities. */
void updateSamplePop (GMM* gmm);

/* Update population for each component based on train data responsibilities. */
void updateTrainPop (GMM* gmm);

/* Update mean for each unfixed component. */
void updateMean (GMM* gmm);

/* Update covariance for each unfixed component. */
void updateCov (GMM* gmm);

/* Update mixing coefficient for each unfixed component. */
void updateMix (GMM* gmm);

/* Update covariance determinant for each unfixed component. */
void updateCovDet (GMM* gmm);

/* Update probability coefficient for each unfixed component. */
void updateProbCoef (GMM* gmm);


#endif
