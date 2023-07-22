/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : threadpool
 * @created     : Saturday Jul 22, 2023 10:28:37 CEST
 */

#ifndef THREADPOOL_H

#define THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>

/**
 * @brief A function running a thread pool task.
 */
typedef struct Threadpool threadpool_t ;

/**
 * @brief A function running a thread pool task.
 */
typedef void (*threadpool_task_t)(void *);

/**
 * @brief A threadpool
 */
struct Threadpool {
	int idle;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	size_t nthreads;
	struct ThreadpoolWorker *workers;
	struct ThreadpoolTask *task_queue;
	bool run;
};

/**
 * @brief Initializes a threadpool.
 */
int threadpool_init(threadpool_t *threadpool, size_t num_threads);

/**
 * @brief Adds a task to the threadpool.
 *
 * @param threadpool The threadpool to add the task to.
 * @param task The task function to run the task
 * @param arg The argument to the task function.
 */
int threadpool_schedule(threadpool_t *threadpool, threadpool_task_t task, void *arg);

/**
 * @brief Cleans up a threadpool.
 */
int threadpool_cleanup(threadpool_t *threadpool);

#endif // THREADPOOL_H
