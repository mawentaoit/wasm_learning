#include <pthread.h>
#include <stdio.h>

int fibonacci(int iterations) {
    int val = 1;
    int last = 0;
    if(iterations == 0) {
        return 0;
    }
    for(int i = 1; i < iterations; i++) {
        int seq;
        seq = val + last;
        last = val;
        val = seq;
    }
    return val;
}

void *bg_func(void* arg) {
    int *iter = (int*) arg;
    *iter = fibonacci(*iter);
    return arg;
}

int main() {
    int fg_val = 54;
    int bg_val = 42;
    pthread_t bg_thread;

    if(pthread_create(&bg_thread, NULL, bg_func, &bg_val)) {
        perror("Thread create failed");
        return 1;
    }

	fg_val=fibonacci(fg_val);
	if(pthread_join(bg_thread,NULL))
	{
		perror("Thread join failed");
		return 2;
	}
	printf("Fib(42) is %d ,Fib(6*9) is %d\n",bg_val,fg_val);
	return 0; 
}