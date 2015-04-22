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
