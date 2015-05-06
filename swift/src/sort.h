/* sort.h */

#ifndef _sort_h
#define _sort_h

/* Sort both index and order descendingly. */
void quickSortDouble (int* index, double* order, int l, int r);
void quickSortInt (int* index, int* order, int l, int r);


int partDouble (int* index, double* order, int l, int r);
int partInt (int* index, int* order, int l, int r);

#endif
