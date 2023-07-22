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
libthreadpool = library(
    'threadpool',
    'threadpool.c',
    install: not meson.is_subproject(),
)
include_dir = include_directories('include')
if not meson.is_subproject()
    install_headers('include/threadpool.h')
endif
libthreadpool_dep = declare_dependency(
    include_directories: include_dir,
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

pkg_mod = import('pkgconfig')
pkg_mod.generate(
    libthreadpool,
    name: 'libthreadpool',
    description: 'A simple thread pool library',
    libraries: libthreadpool,
    version: meson.project_version(),
)
