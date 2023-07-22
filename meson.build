project('libthreadpool', 'c', version: '0.1', default_options: ['warning_level=3'])

threads_dep = dependency('threads')

libthreadpool = library('libthreadpool', 'threadpool.c', install: true)

libthreadpool_dep = declare_dependency(
    include_directories: ['include'],
    dependencies: [threads_dep],
    link_with: [libthreadpool],
)

threadpool_test = executable('threadpool_test', 'threadpool_test.c', dependencies: [libthreadpool_dep])

test('test', threadpool_test)
