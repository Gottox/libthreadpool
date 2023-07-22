#include <stdio.h>
#include <stdlib.h>
#include <threadpool.h>

void task(void *arg) {
	int *num = (int *)arg;
	printf("Hello from task %d\n", *num);
}

int main(void) {
	threadpool_t pool = threadpool_init(0);
	for (int i = 0, store[10]; i < 10; i++) {
		store[i] = i;
		threadpool_schedule(pool, task, &store[i]);
	}
	threadpool_destroy(pool);
}
