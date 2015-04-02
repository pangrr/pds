/* data.h */

#ifndef _data_h
#define _data_h

    typedef struct
    {
        double** data;  /* The data matrix. */
        int samples;    /* Number of data. */   
        int dimension;  /* Number of dimensions of the data. */
    } data;

    data* data_load (char*);

#endif
