all: barriers_test locks_test

barriers_test: barriers_test.cpp barriers.h barriers.cpp atoms.h hrtimer_x86.h hrtimer_x86.c
	g++ -lpthread -Wall barriers_test.cpp barriers.cpp hrtimer_x86.c -o barriers_test

locks_test: locks_test.cpp locks.h locks.c atoms.h hrtimer_x86.h hrtimer_x86.c
	g++ -lpthread -Wall locks_test.cpp locks.c hrtimer_x86.c -o locks_test

clean:
	rm barriers_test locks_test
