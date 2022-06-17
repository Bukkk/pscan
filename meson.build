project('pscan', 'c',
    version : '0.1',
    default_options : [
        'warning_level=3', # Wall Wextra Wpedantic
        'c_std=c11'
    ]
)

build_args = []
link_args = []
deps = []

# zgaduje ze ubuntu ma stare glibc
thread_dep = dependency('threads')
deps += [thread_dep]

if meson.get_compiler('c').get_id() == 'clang'
    build_args += ['-Weverything']
endif

inc = [include_directories('src')]

subdir('src')
subdir('tests')