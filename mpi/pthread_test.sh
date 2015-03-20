#!/bin/bash

g++ -Wall -O3 -lpthread gauss_pthread.cpp hrtimer_x86.c -o gauss_pthread

for thread_num in 2 4 8 16 32
do
  ./gauss_pthread -p $thread_num
done

