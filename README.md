# libthreadpool

A simplistic thread pool implementation in C.

## Functions

### `int threadpool_init(threadpool_t *thread, int num_threads)`

Initializes a thread pool with `num_threads` threads. Returns `0` on success, 
`-1` on failure. If `num_threads` is `0`, the number of threads will be set to
the number of cores on the system.

### `int threadpool_schedule(threadpool_t *thread, threadpool_task_t task, void *arg)`

Schedules a task to be executed by the thread pool. Returns `0` on success, 
`-1` on failure.

### `int threadpool_destroy(threadpool_t *thread)`

Waits for all tasks to finish and destroys the thread pool. Returns `0` on
success, `-1` on failure.

