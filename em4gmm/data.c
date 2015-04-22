/* data.c */

#include "data.h"
#include "global.h"


DataSet* loadData (const char* fileName, int dim, int dataSize)
{
    DataSet* dataSet = (DataSet*) malloc (sizeof(DataSet));
    double* dataPoint;
    int i, j;
    char line[BUFSIZ];
    char* pEnd;
    FILE* file = fopen (fileName, "r");
    
    if (file != NULL)
    {
        /* Initialize the data set. */
        dataSet->size = dataSize;
        dataSet->dim = dim;
        dataSet->dataSet = (double**) malloc (dataSize*sizeof(double*));


        i = 0;
        while (fgets (line, sizeof(line), file))
        {
            /* Initialize a data point. */
            dataSet->dataSet[i] = (double*) malloc (sizeof(double));
            dataPoint = dataSet->dataSet[i];
            
            /* Convert string to double. */
            pEnd = line;
            for (j = 0; j < dim; j++)
            {
                dataPoint[j] = (double)strtod (pEnd, &pEnd);
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
