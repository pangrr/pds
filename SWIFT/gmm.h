#ifndef _gmm_h
#define _gmm_h


    typedef struct
    {
        double mix; /* Mixing coefficient of this component of the GMM. */
        double* mean;   /* Means. */
        double* dcov;   /* Diagonal covariances. */
        double* _mean;  /* Future means. */
        double* _dcov;  /* Future diagonal covariances. */
        int _cfreq; /* Number of data samples clessified to this component. */
    } gauss;


    typedef struct
    {
        gauss* mix; /* Components. */
        int dimension;  /* Dimensions of the GMM. */
        int components; /* Number of components of the GMM. */
        double like;    /* Log likelihood of the GMM on the training data. */
    } gmm;


    double gmm_train (data*, gmm);



#endif
