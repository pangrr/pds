all: gauss_pthread_block_v1.o gauss_pthread_block_v2.o gauss_pthread_cyclic_v1.o gauss_pthread_cyclic_v2.o cmpfile.o

gauss_pthread_block_v1.o : gauss_pthread_block_v1.cpp hrtimer_x86.c hrtimer_x86.h
	g++ -g -Wall -lpthread hrtimer_x86.c gauss_pthread_block_v1.cpp -o gauss_pthread_block_v1.o

gauss_pthread_block_v2.o : gauss_pthread_block_v2.cpp hrtimer_x86.c hrtimer_x86.h
	g++ -g -Wall -lpthread hrtimer_x86.c gauss_pthread_block_v2.cpp -o gauss_pthread_block_v2.o

gauss_pthread_cyclic_v1.o : gauss_pthread_cyclic_v1.cpp hrtimer_x86.c hrtimer_x86.h
	g++ -g -Wall -lpthread hrtimer_x86.c gauss_pthread_cyclic_v1.cpp -o gauss_pthread_cyclic_v1.o

gauss_pthread_cyclic_v2.o : gauss_pthread_cyclic_v2.cpp hrtimer_x86.c hrtimer_x86.h
	g++ -g -Wall -lpthread hrtimer_x86.c gauss_pthread_cyclic_v2.cpp -o gauss_pthread_cyclic_v2.o

cmpfile.o: cmpfile.cpp
	g++ -g -Wall cmpfile.cpp -o cmpfile.o

clean:
	rm *.o
