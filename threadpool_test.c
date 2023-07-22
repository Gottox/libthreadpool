/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         threadpool_test.c
 */

#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <threadpool.h>
#include <unistd.h>

static atomic_int counter = 0;

static void
thread_func_inc(void *arg) {
	(void)arg;
	int rv = 0;

	usleep(1000);
	atomic_fetch_add(&counter, 1);
	assert(rv == PTHREAD_BARRIER_SERIAL_THREAD || rv == 0);
}

static void
test_init_cleanup(void) {
	threadpool_t pool;
	int rv = 0;

	pool = threadpool_init(2);
	assert(rv == 0);

	rv = threadpool_destroy(pool);
	assert(rv == 0);
}

static void
test_add_task(void) {
	threadpool_t pool;
	int rv = 0;
	counter = 0;

	pool = threadpool_init(1);
	assert(pool != NULL);

	rv = threadpool_schedule(pool, thread_func_inc, NULL);
	assert(rv == 0);

	while (atomic_load(&counter) != 1) {
		usleep(1000);
	}

	rv = threadpool_destroy(pool);
	assert(rv == 0);
}

static void
test_add_multiple_tasks(void) {
	struct Threadpool *pool;
	int rv = 0;
	int tasks = 10;

	counter = 0;

	pool = threadpool_init(2);
	assert(pool != NULL);

	for (int i = 0; i < tasks; i++) {
		rv = threadpool_schedule(pool, thread_func_inc, NULL);
		assert(rv == 0);
	}

	while (atomic_load(&counter) != tasks) {
		usleep(1000);
	}

	puts("cleanup");
	rv = threadpool_destroy(pool);
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
