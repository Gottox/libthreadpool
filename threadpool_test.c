/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : test
 * @created     : Saturday Jul 22, 2023 10:20:57 CEST
 */

#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <threadpool.h>
#include <unistd.h>
#include <stdatomic.h>

static atomic_int counter = 0;

static void
thread_func_inc(void *arg) {
	(void)arg;
	int rv = 0;
	char name[16] = {0};
	pthread_getname_np(pthread_self(), name, sizeof(name));

	printf("thread_func_inc %s\n", name);
	usleep(1000);
	atomic_fetch_add(&counter, 1);
	assert(rv == PTHREAD_BARRIER_SERIAL_THREAD || rv == 0);
}

static void
test_init_cleanup(void) {
	struct Threadpool pool = {0};
	int rv = 0;

	rv = threadpool_init(&pool, 2);
	assert(rv == 0);

	rv = threadpool_cleanup(&pool);
	assert(rv == 0);
}

static void
test_add_task(void) {
	struct Threadpool pool = {0};
	int rv = 0;
	counter = 0;

	rv = threadpool_init(&pool, 1);
	assert(rv == 0);

	rv = threadpool_schedule(&pool, thread_func_inc, NULL);
	assert(rv == 0);

	while (atomic_load(&counter) != 1) {
		usleep(1000);
	}

	rv = threadpool_cleanup(&pool);
	assert(rv == 0);
}

static void
test_add_multiple_tasks(void) {
	struct Threadpool pool = {0};
	int rv = 0;
	int tasks = 10;

	counter = 0;

	rv = threadpool_init(&pool, 2);
	assert(rv == 0);

	for (int i = 0; i < tasks; i++) {
		rv = threadpool_schedule(&pool, thread_func_inc, NULL);
		assert(rv == 0);
	}

	while (atomic_load(&counter) != tasks) {
		usleep(1000);
	}

	puts("cleanup");
	rv = threadpool_cleanup(&pool);
	assert(rv == 0);
}

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	fputs("test_init_cleanup", stdout);
	test_init_cleanup();
	fputs(" done\n", stdout);
	fputs("test_add_task", stdout);
	test_add_task();
	fputs(" done\n", stdout);
	fputs("test_add_multiple_tasks", stdout);
	test_add_multiple_tasks();
	fputs(" done\n", stdout);

	return 0;
}
