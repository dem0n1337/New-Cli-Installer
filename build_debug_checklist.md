# CachyOS CLI Installer Build Troubleshooting Checklist

## Primary Goal
Build a release version of the CachyOS Installer with RAID and LVM support to successfully install CachyOS Linux.

## Step 1: Verify Compiler Configuration Files

### Check subprojects/fmt/meson.build
- [ ] Confirm it contains conditional block for Clang that adds:
  ```
  if cpp.get_id() == 'clang'
      fmt_private_cpp_args += ['-stdlib=libc++', '-fstrict-vtable-pointers']
      fmt_interface_cpp_args += ['-stdlib=libc++']
  endif
  ```
- [ ] If missing, edit the file to add this block after the initial `fmt_private_cpp_args = [ ]` and `fmt_interface_cpp_args = [ ]` lines

### Check subprojects/ftxui/meson.build
- [ ] Confirm it contains conditional block for Clang that adds:
  ```
  cpp = meson.get_compiler('cpp')
  ftxui_cpp_args = []
  if cpp.get_id() == 'clang'
      ftxui_cpp_args += ['-stdlib=libc++', '-fstrict-vtable-pointers']
  endif
  ```
- [ ] Verify `cpp_args : ftxui_cpp_args` is added to all three library declarations:
  - ftxui_screen_lib
  - ftxui_dom_lib
  - ftxui_component_lib
- [ ] If missing, edit the file to add these modifications

## Step 2: Check System Libraries & Symlinks

### Verify required libraries
- [ ] Run `ldd build/cachyos-installer` and check for missing libraries
- [ ] Look specifically for:
  - libunwind.so.1
  - libc++.so.1
  - libc++abi.so.1

### Create/repair symlinks if needed
- [ ] If libunwind.so.1 is missing:
  ```
  sudo ln -s /usr/lib/libunwind.so.8 /usr/lib/libunwind.so.1
  ```
- [ ] If libc++.so.1 is missing but a newer version exists:
  ```
  # Find the actual library first
  find /usr/lib -name "libc++.so.*"
  # Then create symlink to the found version
  sudo ln -s /usr/lib/libc++.so.X /usr/lib/libc++.so.1
  ```
- [ ] If libc++abi.so.1 is missing but a newer version exists:
  ```
  # Find the actual library first
  find /usr/lib -name "libc++abi.so.*"
  # Then create symlink to the found version
  sudo ln -s /usr/lib/libc++abi.so.X /usr/lib/libc++abi.so.1
  ```

## Step 3: Rebuild Project as Release Version

### Docker Image Information
- Image ID: eef56869a417
- Base OS: Ubuntu (Jammy)
- Key packages:
  - libc++-18-dev
  - libc++abi-18-dev
  - libcurl4-openssl-dev
  - pkg-config

### Build Release Version Inside Docker Container
```
docker run --rm -v /home/dem0n/Projects/new-cli-installer:/app -w /app eef56869a417 sh -x -c "{ apt-get update && apt-get install -y pkg-config libcurl4-openssl-dev libc++-18-dev libc++abi-18-dev && rm -rf build && meson setup build --buildtype=release && meson compile -C build && meson test -C build; } > /app/build_log.txt 2>&1"
```

### If build succeeds in Docker but fails to run locally
- [ ] Verify all symlinks by running `ldd build/cachyos-installer` again
- [ ] Create any missing symlinks as shown in Step 2

## Step 4: Verify Installation & Execution

- [ ] Run the installer from build directory:
  ```
  cd build
  sudo ./cachyos-installer
  ```
- [ ] If it fails, check output of `ldd cachyos-installer` again for any missing links

## Additional Troubleshooting

If the above steps don't resolve the issue:

1. Check for differences in Docker vs host environment:
   ```
   docker run --rm -v /home/dem0n/Projects/new-cli-installer:/app -w /app eef56869a417 sh -c "ldd /app/build/cachyos-installer"
   ```
   Compare with output of `ldd build/cachyos-installer` on host

2. Review build_log.txt for new error patterns not covered by the existing fixes

3. Check if any other subprojects need similar compiler flag modifications by examining build errors

## Step 5: Test RAID and LVM Functionality

### Prerequisites
- Ensure your system has these packages installed:
  ```
  sudo pacman -S mdadm lvm2
  ```

### Testing RAID Creation
- [ ] Run the installer and navigate to the RAID menu option
- [ ] Create a RAID array with test devices or loopback devices
- [ ] Verify the array appears in `/proc/mdstat`

### Testing LVM Creation
- [ ] Create Volume Group using either regular devices or the RAID array
- [ ] Create Logical Volumes within the Volume Group
- [ ] Format and verify the LVs are usable

### Complete System Installation
- [ ] Use the RAID/LVM configuration to perform a complete CachyOS installation
- [ ] Verify boot configuration is correctly set up for RAID/LVM
- [ ] Test the completed installation boots successfully
