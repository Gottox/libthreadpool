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

#include "include/threadpool.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

struct Threadpool {
	int idle;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	size_t nthreads;
	struct ThreadpoolWorker *workers;
	struct ThreadpoolTask *task_queue;
	bool run;
};

struct ThreadpoolWorker {
	pthread_t tid;
	int rv;
	struct Threadpool *threadpool;
};

struct ThreadpoolTask {
	threadpool_task_t work;
	void *data;
	struct ThreadpoolTask *next;
};

static int
cpu_count(void) {
	long numCPUs = sysconf(_SC_NPROCESSORS_ONLN);
	return (int)numCPUs;
}

static int
worker_consume_single_task(struct ThreadpoolWorker *worker) {
	struct Threadpool *threadpool = worker->threadpool;
	struct ThreadpoolTask *task = threadpool->task_queue;
	threadpool->task_queue = task->next;

	const threadpool_task_t work = task->work;
	void *data = task->data;
	pthread_mutex_unlock(&threadpool->mutex);

	work(data);
	free(task);

	pthread_mutex_lock(&threadpool->mutex);

	return 0;
}

static int
worker_consume_tasks(struct ThreadpoolWorker *worker) {
	int rv = 0;
	struct Threadpool *threadpool = worker->threadpool;
	struct ThreadpoolTask *task = threadpool->task_queue;

	if (task == NULL) {
		goto out;
	}
	while (threadpool->task_queue != NULL) {
		rv = worker_consume_single_task(worker);
		if (rv < 0) {
			goto out;
		}
	}

out:
	return rv;
}

static void *
worker_run(void *data) {
	struct ThreadpoolWorker *worker = data;
	struct Threadpool *threadpool = worker->threadpool;
	int rv = 0;

	rv = pthread_mutex_lock(&threadpool->mutex);
	if (rv < 0) {
		goto out;
	}
	while (true) {
		rv = worker_consume_tasks(worker);
		if (rv < 0) {
			goto out;
		}

		if (threadpool->run == false) {
			break;
		}
		threadpool->idle++;
		rv = pthread_cond_wait(&threadpool->cond, &threadpool->mutex);
	}
out:
	worker->rv = rv;
	pthread_mutex_unlock(&threadpool->mutex);
	return NULL;
}

struct Threadpool *
threadpool_init(size_t nthreads) {
	struct Threadpool *threadpool;
	int rv = 0;
	if (nthreads == 0) {
		nthreads = cpu_count();
	}

	threadpool = calloc(1, sizeof(*threadpool));
	if (threadpool == NULL) {
		goto out;
	}

	rv = pthread_cond_init(&threadpool->cond, NULL);
	if (rv < 0) {
		threadpool = NULL;
		goto out;
	}

	threadpool->nthreads = nthreads;
	threadpool->run = true;
	threadpool->workers = calloc(nthreads, sizeof(*threadpool->workers));
	if (threadpool->workers == NULL) {
		threadpool = NULL;
		goto out;
	}

	for (size_t i = 0; i < nthreads; i++) {
		struct ThreadpoolWorker *worker = &threadpool->workers[i];
		worker->threadpool = threadpool;
		rv = pthread_create(&worker->tid, NULL, worker_run, worker);
		if (rv < 0) {
			threadpool = NULL;
			goto out;
		}
	}

out:
	return threadpool;
}

int
threadpool_schedule(
		struct Threadpool *threadpool, threadpool_task_t work, void *data) {
	int rv = 0;
	struct ThreadpoolTask *task = NULL;
	struct ThreadpoolTask *last = NULL;

	task = calloc(1, sizeof(*task));
	if (task == NULL) {
		rv = -1;
		goto out;
	}

	task->work = work;
	task->data = data;

	rv = pthread_mutex_lock(&threadpool->mutex);
	if (rv < 0) {
		goto out;
	}

	last = threadpool->task_queue;
	if (last == NULL) {
		threadpool->task_queue = task;
	} else {
		while (last->next != NULL) {
			last = last->next;
		}
		last->next = task;
	}

	rv = pthread_cond_signal(&threadpool->cond);
	if (rv < 0) {
		goto out;
	}

	rv = pthread_mutex_unlock(&threadpool->mutex);
out:
	return rv;
}

int
threadpool_destroy(struct Threadpool *threadpool) {
	int rv = 0;
	rv = pthread_mutex_lock(&threadpool->mutex);
	if (rv < 0) {
		goto out;
	}

	threadpool->run = false;
	rv = pthread_cond_broadcast(&threadpool->cond);
	rv = pthread_mutex_unlock(&threadpool->mutex);

	for (size_t i = 0; i < threadpool->nthreads; i++) {
		struct ThreadpoolWorker *worker = &threadpool->workers[i];

		rv = pthread_join(worker->tid, NULL);
		if (rv < 0) {
			goto out;
		}
	}

	pthread_cond_destroy(&threadpool->cond);
	pthread_mutex_destroy(&threadpool->mutex);

	while (threadpool->task_queue != NULL) {
		struct ThreadpoolTask *task = threadpool->task_queue;
		threadpool->task_queue = task->next;
		free(task);
	}

	free(threadpool->workers);
	free(threadpool);
out:
	return rv;
}
