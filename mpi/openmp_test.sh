#!/bin/bash

g++ -Wall -fopenmp gauss_openmp.cpp hrtimer_x86.c -o gauss_openmp
#g++ -Wall gauss_openmp.cpp hrtimer_x86.c -o gauss_openmp

for proc_num in 2 4 8 16 32
do
  ./gauss_openmp -p $proc_num
done

#./gauss_openmp > M
