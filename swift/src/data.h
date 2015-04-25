/* data.h */

#ifndef _data_h
#define _data_h



typedef struct
{
    double** dataSet;  /* An array of pointers to data points. */
    int size; /* Number of data points. */
    int dim;  /* Number of dimensions. */
} DataSet;


/* Read data set from file. */
DataSet* loadData (const char* fileName, int dim, int size);    
/* Print the first and last n lines of data. */
void printData (DataSet* dataSet, int n);


#endif
