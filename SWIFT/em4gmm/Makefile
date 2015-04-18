swift: global.h gmm.h gmm.c linalg.h linalg.c data.h data.c train.c hrtimer_x86.h hrtimer_x86.c
	gcc -lm -Wall -o swift gmm.c linalg.c data.c train.c hrtimer_x86.c
#	gcc -fopenmp -lm -Wall -o swift gmm.c linalg.c data.c train.c hrtimer_x86.c
#	gcc -pg -lm -Wall -o swift gmm.c linalg.c data.c train.c hrtimer_x86.c

clean:
	rm swift
