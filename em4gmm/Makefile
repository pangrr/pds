em4gmm: global.h gmm.h gmm.c linalg.h linalg.c data.h data.c train.c hrtimer_x86.h hrtimer_x86.c
	gcc -lm -Wall -o em4gmm gmm.c linalg.c data.c train.c hrtimer_x86.c
#	gcc -fopenmp -lm -Wall -o em4gmm gmm.c linalg.c data.c train.c hrtimer_x86.c
#	gcc -pg -lm -Wall -o em4gmm gmm.c linalg.c data.c train.c hrtimer_x86.c

clean:
	rm em4gmm
