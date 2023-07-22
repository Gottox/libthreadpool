#include <stdio.h>
#include <stdlib.h>
#include <threadpool.h>

void
task(void *arg) {
	int *num = (int *)arg;
	printf("Hello from task %d\n", *num);
}

int
main(void) {
	threadpool_t pool = threadpool_init(0);
	int *args = calloc(10, sizeof(int));
	for (int i = 0; i < 10; i++) {
		args[i] = i;
		threadpool_schedule(pool, task, &args[i]);
	}
	threadpool_destroy(pool);
	free(args);
}
