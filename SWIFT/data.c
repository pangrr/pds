/* data.c */

#include "data.h"
#include "global.h"


DataSet* loadData (const char* fileName, int dim, int nData)
{
    DataSet* dataSet = (DataSet*) malloc (sizeof(DataSet));
    DataPoint* dataPoint;
    int i, j;
    char line[BUFSIZ];
    char* pEnd;
    FILE* file = fopen (fileName, "r");
    
    if (file != NULL)
    {
        /* Initialize the data set. */
        dataSet->nData = nData;
        dataSet->dim = dim;
        dataSet->data = (DataPoint**) malloc (nData*sizeof(DataPoint*));


        i = 0;
        while (fgets (line, sizeof(line), file))
        {
            /* Initialize a data point. */
            dataSet->data[i] = (DataPoint*) malloc (sizeof(DataPoint));
            dataPoint = dataSet->data[i];
            dataPoint->dim = dim;
            dataPoint->data = (double*) malloc (dim*sizeof(double));

            
            /* Convert string to double. */
            pEnd = line;
            for (j = 0; j < dim; j++)
            {
                dataPoint->data[j] = (double)strtod (pEnd, &pEnd);
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
