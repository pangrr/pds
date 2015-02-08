// assign each thread rows cyclicly
// paralled:
//      find pivot row
//      eliminate each row
// not paralleled:
//      swap rows
//      normalization pivot row


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <math.h> //fabs

#include <fstream> // ifstream
#include <sstream> 
#include <string>
#include <iostream>
#include <algorithm> //std::max

#include "hrtimer_x86.h"

using namespace std;

#define EPSILON 0.00000000001
// global variables
pthread_barrier_t barrier;
pthread_mutex_t mutex_pivot;



double **matrix;
int M, N;
int pivot_row;
double pivot_value;
int thread_num;


void matrix_write(const char *file_name) {
    ofstream outfile(file_name);
    outfile.precision(6);

    if(outfile.is_open()) {
        outfile << M << " " << N << endl;
       
        for(int r = 0; r < M; r++) {
            for(int c = 0; c < N; c++) {
                if(fabs(matrix[r][c]) > EPSILON) {
                    outfile << showpoint << r+1 << " " << c+1  <<  " " << matrix[r][c] << endl;
                }
            }
        }
    }
    outfile << 0;// file end mark
    outfile.close();
}



int matrix_init(const char *file_name) {
    int r, c, n;
    double val;

    ifstream infile(file_name);

    // parse the first line to get the matrix size
    infile >> M >> N >> n;

    // allocate memory for matrix and set all elements to zero
    matrix = new double*[M];
    for(int i = 0; i < M; i++) {
        matrix[i] = new double[N];
    }
    for(int i = 0; i < M; i++) {
        for(int j = 0; j < N; j++) {
            matrix[i][j] = 0.0;
        }
    }
    
    // read the rest of input file to fill the matrix
    while(infile >> r >> c >> val) {
        if(r == 0) break;
        matrix[r-1][c-1] = val;
    }
    return n;
}




int find_local_pivot(int work_row, int begin_row, double *local_pivot_value) {
	int local_pivot_row = begin_row;
    for(int r = begin_row + thread_num; r < M; r += thread_num) {
		if(fabs(matrix[r][work_row]) > fabs(*local_pivot_value)) {
			*local_pivot_value = matrix[r][work_row];
			local_pivot_row = r;
		}
	}
	return local_pivot_row;
}


void rows_swap(int r1, int r2, int start_col) {
	for(int c = start_col; c < N; c++) {
		double temp = matrix[r1][c];
		matrix[r1][c] = matrix[r2][c];
		matrix[r2][c] = temp;
	}
}


void row_norm(int r, int start_c) {
	double deno = matrix[r][start_c];
    for(int c = start_c; c < N; c++) {
		matrix[r][c] /= deno;
	}
}



void row_eliminate(int work_row, int begin_row) {
    int r;
    if(work_row == begin_row) {
        r = begin_row + thread_num;
    } else {
        r = begin_row;
    }

    while(r < M) {
        double multi = matrix[r][work_row];
        for(int c = work_row; c < N; c++) {
            matrix[r][c] -= multi * matrix[work_row][c];
        }
        r += thread_num;
    }
}


void *work_thread(void *lp) {
	// initialize
	int thread_id = *((int *)lp);
	pthread_barrier_wait(&barrier);

	// start gaussian elimination
	for(int work_row = 0; work_row < M; work_row++) {
        int begin_row;
        int work_row_thread = work_row % thread_num;
        if(thread_id >= work_row_thread) {
            begin_row = work_row + thread_id - work_row_thread;
        } else {
            begin_row = work_row + thread_num - work_row_thread + thread_id;
        }

        // initialize pivot row and pivot value
        if(thread_id == 0) {
            pivot_row = work_row;
            pivot_value = matrix[work_row][work_row];
        }
        pthread_barrier_wait(&barrier);

        // find local pivot
        if(begin_row < M) {
            double local_pivot_value = matrix[begin_row][work_row];
            int local_pivot_row  = find_local_pivot(work_row, begin_row,  &local_pivot_value);
                
            // check and update global pivot
            pthread_mutex_lock(&mutex_pivot);
            if(fabs(pivot_value) < fabs(local_pivot_value)) {
                pivot_value = local_pivot_value;
                pivot_row = local_pivot_row;
            }
            pthread_mutex_unlock(&mutex_pivot);
        }
        pthread_barrier_wait(&barrier);
        
        // check singular
        if(thread_id == 0) {
            assert(fabs(pivot_value) > EPSILON);
            // swap pivot row and work row
            rows_swap(work_row, pivot_row, work_row);
            // normalize work row
            row_norm(work_row, work_row);
        }
        pthread_barrier_wait(&barrier);

        // elimination
        if(begin_row < M) {
            row_eliminate(work_row, begin_row);
        }
        pthread_barrier_wait(&barrier);
    }
}


int main(int argc, char *argv[]) {
//    struct timeval start, finish;
    
    thread_num = 1;
    int iterates = 1;
	char *file_name;
	pthread_attr_t attr;
	pthread_t *tid;
	int *id;

	// input arguments
	int c;
	while((c = getopt(argc, argv, "p:f:i:")) != -1) {
		switch(c) {
	case 'p':
			thread_num = atoi(optarg);
			break;
		case 'f':
			file_name = optarg;
			break;
		case 'i':
			iterates = atoi(optarg);
			break;
		}
	}
    
    // read matrix
	matrix_init(file_name);

//    gettimeofday(&start, 0);
    double start = gethrtime_x86();

    for(int it = 0; it < iterates; it++) {
        // initialize pthread
        pthread_barrier_init(&barrier, NULL, thread_num);
        pthread_mutex_init(&mutex_pivot, NULL);
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        // launch threads
        id = (int *)malloc(sizeof(int) * thread_num); 
        assert(id != NULL);
        tid = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
        assert(tid != NULL);
        pthread_attr_init(&attr);
        for(int i = 0; i < thread_num; i++) {
            id[i] = i;
            pthread_create(&tid[i], &attr, work_thread, (void *)&id[i]);
        }

        // join threads
        for(int i = 0; i < thread_num; i++) {
            pthread_join(tid[i], NULL);
        }
    }
//	gettimeofday(&finish, 0);
    double finish = gethrtime_x86();

    matrix_write("v1_cyclic_pthread_result");

//    printf("%f\n", ((finish.tv_sec-start.tv_sec)+(finish.tv_usec-start.tv_usec)*0.000001) / (double)iterates);
    printf("%f\n", (finish-start)/(double)iterates);
	return 0;
}

