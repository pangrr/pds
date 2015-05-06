/* search.c */

#include "search.h"

int biSearch (int x, int* A, int l, int r)
{
    int m;
    if (r - l < 2)
    {
        return -1;
    }
    else
    {
        m = (l + r) / 2;
        if (x < A[m])
        {
            return biSearch (x, A, l, m);
        }
        else if (x > A[m])
        {
            return biSearch (x, A, m, r);
        }
        else
        {
            return m;
        }
    }
}
