project('fmt', 'cpp',
    version : '11.1.4',
    license : 'BSD',
    default_options : [
      'cpp_std=c++20',
      'default_library=static',
    ]
)

fmt_private_cpp_args = [ ]
fmt_interface_cpp_args = [ ]
cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'clang'
    fmt_private_cpp_args += ['-stdlib=libc++', '-fstrict-vtable-pointers']
    fmt_interface_cpp_args += ['-stdlib=libc++']
endif
libtype = get_option('default_library')
if libtype == 'shared'
    fmt_private_cpp_args += [ '-DFMT_EXPORT' ]
    fmt_interface_cpp_args += [ '-DFMT_SHARED' ]
endif

fmt_inc = include_directories('include')
fmt_lib = library('fmt',
    sources : [
        'src/format.cc',
        'src/os.cc'
    ],
    cpp_args : fmt_private_cpp_args,
    include_directories : fmt_inc
)

fmt_dep = declare_dependency(
    include_directories : fmt_inc,
    compile_args : fmt_interface_cpp_args,
    link_with : fmt_lib
)

fmt_header_only_dep = declare_dependency(
    include_directories : fmt_inc,
    compile_args : '-DFMT_HEADER_ONLY'
)
