/* data.c */

#include "data.h"
#include "global.h"


DataSet* loadData (const char* fileName, int dim, int size)
{
    DataSet* dataSet = (DataSet*) malloc (sizeof(DataSet));
    int i, j;
    char line[BUFSIZ];
    char* pEnd;
    FILE* file = fopen (fileName, "r");
    
    if (file != NULL)
    {
        /* Initialize the data set. */
        dataSet->size = size;
        dataSet->dim = dim;
        dataSet->dataSet = (double**) malloc (size*sizeof(double*));


        i = 0;
        while (fgets (line, sizeof(line), file))
        {
            /* Initialize a data point. */
            dataSet->dataSet[i] = (double*) malloc (dim*sizeof(double));
            
            /* Convert string to double. */
            pEnd = line;
            for (j = 0; j < dim; j++)
            {
                dataSet->dataSet[i][j] = (double)strtod (pEnd, &pEnd);
            }
            i++;
        }
    }
    else
    {
        printf ("Fail to load data file!\n");
    }

    fclose (file);
    return dataSet;
}



DataSet* getSubDataSet (DataSet* dataSet, int* subset, int n)
{
    DataSet* subDataSet = (DataSet*) malloc (sizeof(DataSet*));
    subDataSet->dim = dataSet->dim;
    subDataSet->size = n;
    subDataSet->dataSet = (double**) malloc (n*sizeof(double*));
    int i;
    for (i = 0; i < n; i++)
    {
        subDataSet->dataSet[i] = dataSet->dataSet[subset[i]];
//        for (j = 0; j < subDataSet->dim; j++)
//        {
//            printf ("%f ", subDataSet->dataSet[i][j]);
//        }
//        printf ("\n");
    }
    return subDataSet;
}



int* randSample (int nData, int sampleSize)
{
    int* randIndex = (int*) malloc (sampleSize*sizeof(int));
    double* order = (double*) malloc (nData*sizeof(double));
    int* index = (int*) malloc (nData*sizeof(int));
    int i;
    
    for (i = 0; i < nData; i++)
    {
        order[i] = (double)rand()/(double)RAND_MAX;
        index[i] = i;
    }

    quickSortDouble (index, order, 0, nData-1);

    for (i = 0; i < sampleSize; i++)
    {
        randIndex[i] = index[i];
    }

    quickSortInt (index, randIndex, 0, sampleSize-1);

    free (order);
    free (index);
    return randIndex;
}




void printData (DataSet* dataSet, int n)
{
    int dim = dataSet->dim;
    int size = dataSet->size;
    double* dataPoint;
    int i, j;

    if (n*2 < size)
    {
        for (i = 0; i < n; i++)
        {
            dataPoint = dataSet->dataSet[i];
            for (j = 0; j < dim; j++)
            {
                printf ("%f ", dataPoint[j]);
            }
            printf ("\n");
        }

        printf ("...\n");

        for (i = size-n; i < size; i++)
        {
            dataPoint = dataSet->dataSet[i];
            for (j = 0; j < dim; j++)
            {
                printf ("%f ", dataPoint[j]);
            }
            printf ("\n");
        }
    }
    else
    {
        for (i = 0; i < size; i++)
        { 
            dataPoint = dataSet->dataSet[i];
            for (j = 0; j < dim; j++)
            {
                printf ("%f ", dataPoint[j]);
            }
            printf ("\n");
        }
    }
}

