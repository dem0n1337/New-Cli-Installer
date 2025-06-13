cc = meson.get_compiler('cpp')
if cc.get_id() == 'clang'
    # Ensure gucc_lib is also built and linked with libc++ when using Clang,
    # consistent with the main project.
    clang_compile_args = ['-stdlib=libc++', '-fstrict-vtable-pointers']
    clang_link_args = ['-stdlib=libc++']
    add_project_arguments(cc.get_supported_arguments(clang_compile_args), language : 'cpp')
    add_project_link_arguments(cc.get_supported_link_arguments(clang_link_args), language : 'cpp')
endif

gucc_lib = library('gucc',
    sources : [
        'src/io_utils.cpp',
        'src/file_utils.cpp',
        'src/string_utils.cpp',
        'src/fs_utils.cpp',
        'src/cpu.cpp',
        'src/pacmanconf_repo.cpp',
        'src/repos.cpp',
        'src/initcpio.cpp',
        'src/block_devices.cpp',
        'src/partitioning.cpp',
        'src/swap.cpp',
        'src/luks.cpp',
        'src/zfs.cpp',
        'src/btrfs.cpp',
        'src/user.cpp',
        'src/locale.cpp',
        'src/fstab.cpp',
        'src/crypttab.cpp',
        'src/bootloader.cpp',
        'src/systemd_services.cpp',
        'src/autologin.cpp',
        'src/mtab.cpp',
        'src/umount_partitions.cpp',
        'src/mount_partitions.cpp',
        'src/hwclock.cpp',
        'src/package_profiles.cpp',
        'src/logger.cpp',
        'src/fetch_file.cpp',
        'src/package_list.cpp',
        'src/kernel_params.cpp',
        'src/chwd.cpp',
    ],
    include_directories : [include_directories('include')],
    dependencies: deps
)

if is_tests_build
    subdir('tests')
endif
