/* data.h */

#ifndef _data_h
#define _data_h



typedef struct
{
    double* data;
    int dim;
} DataPoint;



typedef struct
{
    DataPoint** data;  /* An array of pointers to data points. */
    int nData; /* Number of data points. */
    int dim;  /* Number of dimensions. */
} DataSet;


/* Read data set from file. */
DataSet* loadData (const char* fileName, int dim, int nData);    



#endif
