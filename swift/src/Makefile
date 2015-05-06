swift: global.h gmm.h gmm.c kmeans.h kmeans.c data.h data.c search.h search.c sort.h sort.c main.c hrtimer_x86.h hrtimer_x86.c
#	gcc -lm -Wall -o swift gmm.c kmeans.c data.c search.c sort.c main.c hrtimer_x86.c
	gcc -fopenmp -lm -Wall -o swift gmm.c kmeans.c data.c search.c sort.c main.c hrtimer_x86.c
#	gcc -pg -lm -Wall -o swift gmm.c kmeans.c data.c search.c sort.c main.c hrtimer_x86.c

clean:
	rm swift
