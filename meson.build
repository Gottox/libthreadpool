project(
    'libthreadpool',
    'c',
    version: '0.1.1',
    default_options: [
        'c_std=c99',
        'optimization=0',
        'warning_level=3',
    ],
)

threads_dep = dependency('threads')
libthreadpool = library('libthreadpool', 'threadpool.c', install: true)
libthreadpool_dep = declare_dependency(
    include_directories: ['include'],
    dependencies: [threads_dep],
    link_with: [libthreadpool],
)

example_test = executable(
    'example',
    'example.c',
    dependencies: [libthreadpool_dep],
)
test('example', example_test)

threadpool_test = executable(
    'threadpool_test',
    'threadpool_test.c',
    dependencies: [libthreadpool_dep],
)
test('test', threadpool_test)


