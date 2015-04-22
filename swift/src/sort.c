/* sort.c */


#include "global.h"
#include "sort.h"





void quickSort (int* index, double* order, int l, int r)
{
    int i;
    if (l < r)
    {
        i = partition (index, order, l, r);
        quickSort (index, order, l, i-1);
        quickSort (index, order, i+1, r);
    }
}




int partition (int* index, double* order, int l, int r)
{
    int i, j, t2;
    double t1;
    double pivot = order[l];
    i = l;
    j = r + 1;

    while (1)
    {
        do
        {
            ++i;
        }
        while (order[i] <= pivot && i <= r);

        do
        {
            --j;
        }
        while (order[j] > pivot);

        if (i >= j)
        {
            break;
        }
        
        t1 = order[i];
        t2 = index[i];
        order[i] = order[j];
        index[i] = index[j];
        order[j] = t1;
        index[j] = t2;
    }
    t1 = order[l];
    t2 = index[l];
    order[l] = order[j];
    index[l] = index[j];
    order[j] = t1;
    index[j] = t2;
    return j;
}
